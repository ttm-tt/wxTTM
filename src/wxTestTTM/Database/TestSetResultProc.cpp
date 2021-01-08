#include "pch.h"
#include "CppUnitTest.h"

#include "TTDbse.h"

#include "NaStore.h"
#include "PlStore.h"
#include "LtStore.h"
#include "CpStore.h"
#include "GrStore.h"
#include "StStore.h"
#include "MtStore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
  extern wxString cwd;

	namespace Fixtures {
		extern wxString nations[];
		extern wxString players[];
		extern wxString events[];
		extern wxString groups[];
		extern wxString entries[];
		extern wxString positions[];
	}

  TEST_CLASS(TestSetResultProc)
  {
	  private:
			static bool ImportFixture(const wxString data[], bool (*func)(wxTextBuffer &))
			{
				wxMemoryText is;
				for (int i = 0; !data[i].IsEmpty(); i++)
					is.AddLine(data[i]);

				return func(is);
			}

		// Run once before any methods in this class
		TEST_METHOD_INITIALIZE(SetUpMethodSetResultProc)
		{
	    wxSetWorkingDirectory(cwd);

			// Reset DB to known state
			wxFileName tmpName("TTM_UTEST", "TTM_UTEST-v153.bak");
			tmpName.Normalize();
  
			bool ret = TTDbse::instance()->RestoreDatabase(tmpName.GetFullPath(), "TTM_UTEST");
			Assert::IsTrue(ret);		

			// Add fixtures
			Assert::IsTrue(ImportFixture(Fixtures::nations, NaStore::Import));
			Assert::IsTrue(ImportFixture(Fixtures::players, PlStore::Import));
			Assert::IsTrue(ImportFixture(Fixtures::events, CpStore::Import));
			Assert::IsTrue(ImportFixture(Fixtures::groups, GrStore::Import));
			Assert::IsTrue(ImportFixture(Fixtures::entries, LtStore::Import));
			Assert::IsTrue(ImportFixture(Fixtures::positions, StStore::Import));
		}

		TEST_METHOD(Ó_000_Initial)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			Assert::AreNotEqual(0L, gr.grID);
			Assert::AreEqual(cp.cpID, gr.cpID);

			MtStore mt;
			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsFalse(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}
		}

		TEST_METHOD(O_010_SetResult)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			MtStore::MtEvent mtEvent;
			mtEvent.grID = gr.grID;
			mtEvent.mtRound = 1;
			mtEvent.mtMatch = 1;
			mtEvent.mtMS = 0;
			mtEvent.mtChance = 0;

			MtStore mt;
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();

			// Set a normal result
			Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
			Statement *tmp = connPtr->CreateStatement();

			wxString sql = wxString::Format("mtSetResultProc %d, 0, 5, '110811071106', 0, 0, 0, 0, 0, 0", mt.mtNr);
			tmp->ExecuteUpdate(sql);

			// They still did not proceeed in the draw
			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsFalse(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}

			// Verify the result is 3:0
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();
			Assert::AreEqual((short) 3, mt.mtResA);
			Assert::AreEqual((short) 0, mt.mtResX);
			
			Assert::IsTrue(mt.UpdateScoreChecked(mt.mtID, true));

			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsTrue(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}
		}

		TEST_METHOD(O_011_SetResultTiebreak)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			// Change 5th game to play to 6
			cp.cpPtsToWinLast = 6;
			cp.cpPtsAheadLast = 1;
			cp.Update();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			MtStore::MtEvent mtEvent;
			mtEvent.grID = gr.grID;
			mtEvent.mtRound = 1;
			mtEvent.mtMatch = 1;
			mtEvent.mtMS = 0;
			mtEvent.mtChance = 0;

			MtStore mt;
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();

			// Set a normal result
			Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
			Statement *tmp = connPtr->CreateStatement();

			wxString sql = wxString::Format("mtSetResultProc %d, 0, 5, '11081107061107110605', 0, 0, 0, 0, 0, 0", mt.mtNr);
			tmp->ExecuteUpdate(sql);

			// They still did not proceeed in the draw
			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsFalse(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}

			// Verify the result is 3:2
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();
			Assert::AreEqual((short) 3, mt.mtResA);
			Assert::AreEqual((short) 2, mt.mtResX);
			
			Assert::IsTrue(mt.UpdateScoreChecked(mt.mtID, true));

			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsTrue(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}

		}

		TEST_METHOD(O_020_SetResultWithWOX)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			MtStore::MtEvent mtEvent;
			mtEvent.grID = gr.grID;
			mtEvent.mtRound = 1;
			mtEvent.mtMatch = 1;
			mtEvent.mtMS = 0;
			mtEvent.mtChance = 0;

			MtStore mt;
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();

			// Set a normal result
			Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
			Statement *tmp = connPtr->CreateStatement();

			wxString sql = wxString::Format("mtSetResultProc %d, 0, 5, '110011001100', 0, 1, 0, 0, 0, 0", mt.mtNr);
			tmp->ExecuteUpdate(sql);

			// Verify the result is 3:0 w/o
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();
			Assert::AreEqual((short) 3, mt.mtResA);
			Assert::AreEqual((short) 0, mt.mtResX);
			Assert::IsTrue(mt.mtWalkOverX);
		}

		TEST_METHOD(O_030_SetResultWithInjX)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			MtStore::MtEvent mtEvent;
			mtEvent.grID = gr.grID;
			mtEvent.mtRound = 1;
			mtEvent.mtMatch = 1;
			mtEvent.mtMS = 0;
			mtEvent.mtChance = 0;

			MtStore mt;
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();

			// Set a normal result
			Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
			Statement *tmp = connPtr->CreateStatement();

			wxString sql = wxString::Format("mtSetResultProc %d, 0, 5, '110811071103', 0, 0, 0, 1, 0, 0", mt.mtNr);
			tmp->ExecuteUpdate(sql);

			// Verify the result is 3:0 w/o
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();
			Assert::AreEqual((short) 3, mt.mtResA);
			Assert::AreEqual((short) 0, mt.mtResX);
			Assert::IsTrue(mt.mtInjuredX);
		}

		TEST_METHOD(O_040_SetResultWithDisquX)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			MtStore::MtEvent mtEvent;
			mtEvent.grID = gr.grID;
			mtEvent.mtRound = 1;
			mtEvent.mtMatch = 1;
			mtEvent.mtMS = 0;
			mtEvent.mtChance = 0;

			MtStore mt;
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();

			// Set a normal result
			Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
			Statement *tmp = connPtr->CreateStatement();

			wxString sql = wxString::Format("mtSetResultProc %d, 0, 5, 11001100110000000000, 0, 0, 0, 0, 0, 1", mt.mtNr);
			tmp->ExecuteUpdate(sql);

			// Verify the result is 3:0 w/o
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();
			Assert::AreEqual((short) 3, mt.mtResA);
			Assert::AreEqual((short) 0, mt.mtResX);
			Assert::IsTrue(mt.mtDisqualifiedX);
		}

		TEST_METHOD(O_050_SetIncompleteResult)
		{
		  CpStore cp;
			cp.SelectAll();
			cp.Next();
			cp.Close();

			Assert::AreNotEqual(0L, cp.cpID);

			GrStore gr;
			gr.SelectAll(cp);
			gr.Next();
			gr.Close();

			MtStore::MtEvent mtEvent;
			mtEvent.grID = gr.grID;
			mtEvent.mtRound = 1;
			mtEvent.mtMatch = 1;
			mtEvent.mtMS = 0;
			mtEvent.mtChance = 0;

			MtStore mt;
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();

			// Set a normal result
			Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
			Statement *tmp = connPtr->CreateStatement();

			wxString sql = wxString::Format("mtSetResultProc %d, 0, 5, '11081106', 0, 0, 0, 0, 0, 1", mt.mtNr);
			tmp->ExecuteUpdate(sql);

		  // They still did not proceeed in the draw
			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsFalse(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}

			// Verify the result is 2:0
			mt.SelectByEvent(mtEvent);
			mt.Next();
			mt.Close();
			Assert::AreEqual((short) 2, mt.mtResA);
			Assert::AreEqual((short) 0, mt.mtResX);
			
			Assert::IsTrue(mt.UpdateScoreChecked(mt.mtID, true));

			mt.SelectByGr(gr);
			while (mt.Next())
			{
				switch (mt.mtEvent.mtRound)
				{
				  case 1 :
						Assert::IsTrue(mt.stA);
						Assert::IsTrue(mt.stX);
						break;
					case 2 :
						Assert::IsFalse(mt.stA);
						Assert::IsFalse(mt.stX);
						break;
				}
			}
	}

		TEST_METHOD(O_999_Dummy)
		{
			Assert::IsTrue(true);
		}
  };
}