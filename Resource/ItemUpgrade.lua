----------------------------------------------------------
-- 피어싱 확률( n/10000 ) --------------------------------
----------------------------------------------------------
tSuitProb = { 8000, 5000, 2000, 500 }
tWeaponProb = { 5000, 2500, 1250, 625, 313, 156, 78, 39, 20, 10 }


----------------------------------------------------------
-- 일반제련 확률 -----------------------------------------
----------------------------------------------------------
-- 해외는 +4부터 아래 값에서 10% 감소된 값이 적용된다. ( n * 0.9 )
tGeneral = { 10000, 10000, 7000, 5000, 4000, 3000, 2000, 1000, 500, 200 };


----------------------------------------------------------
-- 속성제련 ----------------------------------------------
----------------------------------------------------------
tAttribute = {}
function AddAttribute( nNum, nProb, nDamageRate, nDefenseRate, nAddAtkDmgRate )
	tAttribute[nNum] = {}
	tAttribute[nNum].nProb = nProb
	tAttribute[nNum].nDamageRate = nDamageRate
	tAttribute[nNum].nDefenseRate = nDefenseRate
	tAttribute[nNum].nAddAtkDmgRate = nAddAtkDmgRate
end
--		제련값,		확률,		데미지증가율,		방어력증가율,		속성보정증가율  (제련값을 제외한 모든값은 n/10000)
AddAttribute(	1,		10000,		200,			200,			500		)
AddAttribute(	2,		10000,		221,			221,			522		)
AddAttribute(	3,		9200,		256,			256,			560		)
AddAttribute(	4,		7000,		305,			305,			612		)
AddAttribute(	5,		5300,		368,			368,			680		)
AddAttribute(	6,		4000,		446,			446,			763		)
AddAttribute(	7,		3000,		537,			537,			860		)
AddAttribute(	8,		2300,		642,			642,			973		)
AddAttribute(	9,		1700,		761,			761,			1101		)
AddAttribute(	10,		1300,		895,			895,			1244		)
AddAttribute(	11,		1000,		1042,			1042,			1402		)
AddAttribute(	12,		700,		1203,			1203,			1575		)
AddAttribute(	13,		600,		1379,			1379,			1763		)
AddAttribute(	14,		400,		1568,			1568,			1966		)
AddAttribute(	15,		300,		1772,			1772,			2184		)
AddAttribute(	16,		250,		1989,			1989,			2417		)
AddAttribute(	17,		200,		2221,			2221,			2665		)
AddAttribute(	18,		150,		2467,			2467,			2998		)
AddAttribute(	19,		120,		2726,			2726,			3207		)
AddAttribute(	20,		100,		3000,			3000,			3500		)

----------------------------------------------------------
-- 아이템 트랜지 ----------------------------------------------
----------------------------------------------------------
nItemTransyLowLevel	= 500000
nItemTransyHighLevel	= 2000000