#include "pch.h"

namespace wxTestTTM
{
  namespace Fixtures
	{
		// Fixtures
		wxString nations[] = {
			"NATIONS 1",
			"AAA; Aaa;",
			"BBB; Bbb;",
			"CCC; Ccc;",
			"DDD; Ddd;",
			""
		};

		wxString events[] = {
			"#EVENTS 1",
			"#Name;Description;Type;Sex;Year",
			"MS;Men's Singles;S;M;0",
			""
		};

		wxString groups[] = {
			"#GROUPS 1",
			"# CP; Name; Description; Stage; Modus; Size; Best of; Winner; Group Modus; Team System; Nof Rounds; Nof Matches; 3d Place; ",
			"MS;CP;Main Draw;Championship;KO;4;5;1;;;;;",
			""
		};

		// PlNo: Sex (1/2) Cat (a number) running number
		wxString players[] = {
			"#PLAYERS 1",
			"# No.; Last Name; Given Name; Sex; Born; Assoc; Ext. ID; Ranking Pts.",
			"1001;Aal;Aag;M;0;AAA;;0;",
			"1002;Bal;Bag;M;0;BBB;;0;",
			"1003;Cal;Cag;M;0;BBB;;0;",
			"1004;Dal;Dag;M;0;DDD;;0;",
			""
		};

		wxString entries[] = {
			"#ENTRIES 1",
			"# Pl. No.; Event; Partner No.; Association; Natl. Ranking; Int'l Ranking",
			"1001;MS;;AAA;0;0",
			"1002;MS;;BBB;0;0",
			"1003;MS;;CCC;0;0",
			"1004;MS;;DDD;0;0",
			""
		};

		wxString positions[] = {
			"#POSITIONS 1",
			"# Event; Group; Group Pos.; Pl. No; Seeded; Final Position",
			"MS;CP;1;1001;0;0",
			"MS;CP;2;1002;0;0",
			"MS;CP;3;1003;0;0",
			"MS;CP;4;1004;0;0",
			""
		};

		wxString schedules[] = {
			"#SCHEDULES 1",
			"# Event; Group; Round; Match; Chance; Date; Time; Table; Umpire; Assistant Umpire",
			""
		};

		wxString results[] = {
			"#RESULTS 1",
			"# Event; Group; Round; Match; Team Match; Result A; Result X; Games A; Games X; ",
			""
		};
	}
}