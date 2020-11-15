#include "pch.h"
#include "CppUnitTest.h"

#include "GrStore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace wxTestTTM
{
	TEST_CLASS(TestGrStore)
	{
	public:
		
		TEST_METHOD(TestNofRounds)
		{
		  // RR group of 4
			{
				GrRec gr;
				gr.grModus = MOD_RR;
				gr.grSize = 4;

				Assert::AreEqual((short) 3, gr.NofRounds(false));
		  }

			// KO Group of 4
			{
				GrRec gr;
				gr.grModus = MOD_SKO;
				gr.grSize = 4;

				Assert::AreEqual((short) 2, gr.NofRounds(false));
		  }
		}
	};
}
