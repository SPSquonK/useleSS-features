#include "StdAfx.h"
#include "WndManager.h"
#include "defineText.h"
#include "playerdata.h"

CString SingleDstToString(const SINGLE_DST & singleDst);

namespace WndMgr {

	DWORD CTooltipBuilder::GetOkOrErrorColor(bool isOk) const {
		if (isOk) {
			return dwItemColor[g_Option.m_nToolTipText].dwGeneral;
		} else {
			return dwItemColor[g_Option.m_nToolTipText].dwNotUse;
		}
	}

	void CTooltipBuilder::PutCoolTime(const CMover & pMover, const ItemProp & itemProp, CEditString & pEdit) const {
		const auto remainingCd = pMover.m_cooltimeMgr.GetRemainingTime(itemProp);
		if (remainingCd == 0) return;

		CTimeSpan ct((remainingCd + 500) / 1000); // 남은시간을 초단위로 변환해서 넘겨줌, +500 반올림 

		CString strTemp;
		strTemp.Format(prj.GetText(TID_TOOLTIP_COOLTIME), ct.GetMinutes(), ct.GetSeconds());		// 남은시간을 분/초 형태로 출력.
		pEdit.AddString("\n");
		pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwGeneral);
	}

	void CTooltipBuilder::PutKeepTime(CItemElem & pItemElem, const ItemProp & itemProp, CEditString & pEdit) const {
		CString strTemp;
		if (itemProp.dwCircleTime != NULL_ID) {
			pEdit.AddString("\n");

			if (itemProp.dwCircleTime == 1) {
				pEdit.AddString(prj.GetText(TID_GAME_COND_USE), dwItemColor[g_Option.m_nToolTipText].dwTime); // 사망시 효과 적용
			} else {
				CTimeSpan ct(itemProp.dwCircleTime);
				strTemp.Format("%s : ", prj.GetText(TID_TOOLTIP_ITEMTIME));	// 지속시간 : 
				pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwTime);
				strTemp.Format(prj.GetText(TID_TOOLTIP_DATE), static_cast<int>(ct.GetDays()), ct.GetHours(), ct.GetMinutes(), ct.GetSeconds());	// 지속시간 : 
				pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwTime);
			}
		}

		time_t t = pItemElem.m_dwKeepTime - time_null();

		//비스 남은 시간 표시( 인벤에 있는,, 아직 사용전이므로 서버데이터가 없다. Prop에서 가져다 쓴다 ( 단위는 분 ) 2009_11_10 
		if (t <= 0 && itemProp.IsVis())		// 장착된 비스만 유지시간을 받게 되고, 인벤에 위치한 비스는 Prop에서 가져다 써야 함.
		{
			pItemElem.m_dwKeepTime = itemProp.dwAbilityMin;
			t = (time_t)(pItemElem.m_dwKeepTime * 60.0f);		//분단위 로 바뀜 
		}

		if (pItemElem.m_dwKeepTime && !pItemElem.IsFlag(CItemElem::expired)) {
			CString str;

			if (t > 0) {
				CTimeSpan time(t);
				if (itemProp.dwItemKind3 == IK3_TICKET) {
					str.Format(prj.GetText(TID_TOOLTIP_DATE), static_cast<int>(time.GetDays()), time.GetHours(), time.GetMinutes(), time.GetSeconds());
				} else {
					if (itemProp.IsVis())		// gmpbigsun : 일, 시간, 분, 초 의 텍스트를 읽어서 알맞게 조합해준다.
					{
						CString strDays, strHours, strMinutes, strSeconds;
						if (time.GetDays()) {
							strDays.Format(prj.GetText(TID_PK_LIMIT_DAY), static_cast<int>(time.GetDays()));
							strHours.Format(prj.GetText(TID_PK_LIMIT_HOUR), time.GetHours());
							strMinutes.Format(prj.GetText(TID_PK_LIMIT_MINUTE), time.GetMinutes());

							str = strDays + strHours + strMinutes;
						} else if (time.GetHours()) {
							strHours.Format(prj.GetText(TID_PK_LIMIT_HOUR), time.GetHours());
							strMinutes.Format(prj.GetText(TID_PK_LIMIT_MINUTE), time.GetMinutes());

							str = strHours + strMinutes;
						} else if (time.GetMinutes()) {
							strMinutes.Format(prj.GetText(TID_PK_LIMIT_MINUTE), time.GetMinutes());
							strSeconds.Format(prj.GetText(TID_PK_LIMIT_SECOND), time.GetSeconds());

							str = strMinutes + strSeconds;
						} else {
							str.Format(prj.GetText(TID_PK_LIMIT_SECOND), time.GetSeconds());
						}
					} else if (time.GetDays())
						str.Format(prj.GetText(TID_PK_LIMIT_DAY), static_cast<int>(time.GetDays() + 1));
					else if (time.GetHours())
						str.Format(prj.GetText(TID_PK_LIMIT_HOUR), time.GetHours());
					else if (time.GetMinutes())
						str.Format(prj.GetText(TID_PK_LIMIT_MINUTE), time.GetMinutes());
					else
						str.Format(prj.GetText(TID_PK_LIMIT_SECOND), time.GetSeconds());
				}
			}
			strTemp = str + prj.GetText(TID_TOOLTIP_PERIOD);
			pEdit.AddString("\n");
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwTime);
		}
	}

	void CTooltipBuilder::PutCommand(const CItemElem & pItemElem, CEditString & pEdit) const {
		const ItemProp * itemProp = pItemElem.GetProp();
		assert(itemProp);

		if (std::strlen(itemProp->szCommand) == 0) return;

		pEdit.AddString("\n");

		static constexpr auto CoupleRingIds = {
			II_GEN_WARP_COUPLERING, II_GEN_WARP_WEDDING_BAND, II_GEN_WARP_COUPLERING01
		};

		CString strTemp;
		if (std::ranges::find(CoupleRingIds, pItemElem.m_dwItemId) != CoupleRingIds.end()
			&& pItemElem.GetRandomOptItemId() > 0) {
			u_long idPlayer = (u_long)(pItemElem.GetRandomOptItemId());
			const char * pszPlayer = CPlayerDataCenter::GetInstance()->GetPlayerString(idPlayer);
			CString strDesc;
			strDesc.Format(prj.GetText(TID_ITEM_COUPLERING_DESC), pszPlayer ? pszPlayer : "");
			strTemp.Format(prj.GetText(TID_ITEM_INFO), strDesc);	// 설명 :
		} else {
			strTemp.Format(prj.GetText(TID_ITEM_INFO), itemProp->szCommand);	// 설명 :
		}

		pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwCommand);
	}

	void CTooltipBuilder::PutSex(const CMover & pMover, const ItemProp & itemProp, CEditString & pEdit) const {
		// TODO: maybe add automorph? Gendered items are super inconvenient and
		// bring no value to the gameplay.
		if (itemProp.dwItemSex == NULL_ID) return;

		LPCTSTR sexText;
		if (itemProp.dwItemSex == SEX_MALE) {
			sexText = prj.GetText(TID_GAME_TOOLTIP_SEXMALE);
		} else {
			sexText = prj.GetText(TID_GAME_TOOLTIP_SEXFEMALE);
		}

		// TODO: itemProp.dwItemSex and pMover.GetSex() should have the same type

		const DWORD sexColor = GetOkOrErrorColor(itemProp.dwItemSex == pMover.GetSex());

		pEdit.AddString("\n");
		pEdit.AddString(sexText, sexColor);
	}

	void CTooltipBuilder::PutJob(const CMover & pMover, const ItemProp & itemProp, CEditString & pEdit) const {
		if (itemProp.dwItemJob == NULL_ID) return;

		constexpr auto GetTooltipIdForJob = [](const DWORD jobId) {
			switch (jobId) {
				case JOB_VAGRANT:             return TID_GAME_TOOLTIP_REGVANG;
				case JOB_MERCENARY:           return TID_GAME_TOOLTIP_REGMERSER;
				case JOB_ACROBAT:             return TID_GAME_TOOLTIP_ACRO;
				case JOB_ASSIST:              return TID_GAME_TOOLTIP_ASSIST;
				case JOB_MAGICIAN:            return TID_GAME_TOOLTIP_MAG;
				case JOB_PUPPETEER:           return TID_GAME_TOOLTIP_PUPPET;
				case JOB_KNIGHT:              return TID_GAME_TOOLTIP_KNIGHT;
				case JOB_BLADE:               return TID_GAME_TOOLTIP_BLADE;
				case JOB_JESTER:              return TID_GAME_TOOLTIP_JASTER;
				case JOB_RANGER:              return TID_GAME_TOOLTIP_RANGER;
				case JOB_RINGMASTER:          return TID_GAME_TOOLTIP_RINGMAS;
				case JOB_BILLPOSTER:          return TID_GAME_TOOLTIP_BILLPOS;
				case JOB_PSYCHIKEEPER:        return TID_GAME_TOOLTIP_PSYCHIKEEPER;
				case JOB_ELEMENTOR:           return TID_GAME_TOOLTIP_ELEMENTOR;
				case JOB_GATEKEEPER:          return TID_GAME_TOOLTIP_GATE;
				case JOB_DOPPLER:             return TID_GAME_TOOLTIP_DOPPLER;
				case JOB_KNIGHT_MASTER:       return TID_GAME_TOOLTIP_KNIGHT_MASTER;
				case JOB_BLADE_MASTER:        return TID_GAME_TOOLTIP_BLADE_MASTER;
				case JOB_JESTER_MASTER:       return TID_GAME_TOOLTIP_JESTER_MASTER;
				case JOB_RANGER_MASTER:       return TID_GAME_TOOLTIP_RANGER_MASTER;
				case JOB_RINGMASTER_MASTER:   return TID_GAME_TOOLTIP_RINGMASTER_MASTER;
				case JOB_BILLPOSTER_MASTER:   return TID_GAME_TOOLTIP_BILLPOSTER_MASTER;
				case JOB_PSYCHIKEEPER_MASTER: return TID_GAME_TOOLTIP_PSYCHIKEEPER_MASTER;
				case JOB_ELEMENTOR_MASTER:    return TID_GAME_TOOLTIP_ELEMENTOR_MASTER;
				case JOB_KNIGHT_HERO:         return TID_GAME_TOOLTIP_KNIGHT_HERO;
				case JOB_BLADE_HERO:          return TID_GAME_TOOLTIP_BLADE_HERO;
				case JOB_JESTER_HERO:         return TID_GAME_TOOLTIP_JESTER_HERO;
				case JOB_RANGER_HERO:         return TID_GAME_TOOLTIP_RANGER_HERO;
				case JOB_RINGMASTER_HERO:     return TID_GAME_TOOLTIP_RINGMASTER_HERO;
				case JOB_BILLPOSTER_HERO:     return TID_GAME_TOOLTIP_BILLPOSTER_HERO;
				case JOB_PSYCHIKEEPER_HERO:   return TID_GAME_TOOLTIP_PSYCHIKEEPER_HERO;
				case JOB_ELEMENTOR_HERO:      return TID_GAME_TOOLTIP_ELEMENTOR_HERO;
				default: return 0;
			}
		};

		const DWORD tooltipId = GetTooltipIdForJob(itemProp.dwItemJob);
		const LPCTSTR tooltipText = tooltipId != 0 ? prj.GetText(tooltipId) : "Unknown job";

		const DWORD tooltipColor = GetOkOrErrorColor(pMover.IsInteriorityJob(itemProp.dwItemJob));

		pEdit.AddString("\n");
		pEdit.AddString(tooltipText, tooltipColor);
	}

	void CTooltipBuilder::PutLevel(const CMover & pMover, const CItemElem & pItemElem, CEditString & pEdit) const {
		const DWORD limitLevel1 = pItemElem.GetProp()->dwLimitLevel1;
		if (limitLevel1 == NULL_ID) return;
		
		CString strTemp;
		strTemp.Format(prj.GetText(TID_GAME_TOOLTIP_REQLEVEL), limitLevel1);

		pEdit.AddString("\n");
		
		const DWORD limitLevelColor = GetOkOrErrorColor(!pItemElem.IsLimitLevel(&pMover));
		pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwNotUse);

		if (pItemElem.GetLevelDown() != 0) {
			strTemp.Format("(%d)", pItemElem.GetLevelDown());
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwNotUse);
		}
	}

	void CTooltipBuilder::PutWeapon(const ItemProp & pItemProp, CEditString & pEdit) const {
		if (pItemProp.dwItemKind3 == IK3_SHIELD) return;
		if (pItemProp.dwHanded == NULL_ID) return;
		
		LPCTSTR strTemp = "";
		if (pItemProp.dwHanded == HD_ONE) {
			strTemp = prj.GetText(TID_GAME_TOOLTIP_ONEHANDWEAPON);
		} else if (pItemProp.dwHanded == HD_TWO) {
			strTemp = prj.GetText(TID_GAME_TOOLTIP_TWOHANDWEAPON);
		}

		pEdit.AddString("\n");
		pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwGeneral);
	}

	void CTooltipBuilder::PutAddedOpt(const CItemElem & pItemElem, CEditString & pEdit) const {
		const std::map<int, int> mapDst = prj.m_UltimateWeapon.GetDestParamUltimate(&pItemElem);

		for (const auto & [nDst, nAdj] : mapDst) {
			const CString strTemp = "\n" + SINGLE_DST{nDst, nAdj}.ToString();
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwAddedOpt[6]);
		}
	}

	void CTooltipBuilder::PutPetInfo(const CItemElem & pItemElem, CEditString & pEdit) const {
		pEdit.Empty();

		//Name
		CString strTemp = pItemElem.GetName();
		pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwName0, ESSTY_BOLD);

		PutPetKind(pItemElem, pEdit);

		// Alive?
		if (pItemElem.IsFlag(CItemElem::expired)) {
			strTemp.Format(" %s", prj.GetText(TID_GAME_PETINFO_DEAD));
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwResistSM1, ESSTY_BOLD);
		}

		//Level
		if (pItemElem.m_pPet) {
			const PETLEVEL nLevel = pItemElem.m_pPet->GetPetLevel();
			const DWORD dwLevelText = CPetProperty::GetTIdOfLevel(nLevel);

			strTemp.Format("%s : %s", prj.GetText(TID_GAME_CHARACTER_02), prj.GetText(dwLevelText));
			pEdit.AddString("\n");
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwResistSM);

			//Ability value
			if (nLevel != PL_EGG) {
				const SINGLE_DST dst = pItemElem.m_pPet->GetAvailDestParam();
				const DWORD dwTooltip = CPetProperty::GetTIdOfDst(dst);

				strTemp.Format("%s : %s +%d", prj.GetText(TID_GAME_ABILITY), prj.GetText(dwTooltip), dst.nAdj);
				pEdit.AddString("\n");
				pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwResistSM1);
			}
			//Level History
			if (nLevel > PL_EGG) {
				pEdit.AddString("\n");
				pEdit.AddString("(", D3DCOLOR_XRGB(0, 200, 255));
				for (int i = PL_D; i <= nLevel; i++) {
					BYTE bLevel = pItemElem.m_pPet->GetAvailLevel(i);
					strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_LEVEL), bLevel);
					pEdit.AddString(strTemp, D3DCOLOR_XRGB(0, 127, 255));
					if (i != nLevel)
						pEdit.AddString("/", D3DCOLOR_XRGB(0, 200, 255));
				}
				pEdit.AddString(")", D3DCOLOR_XRGB(0, 200, 255));

				//Pet Experience
				EXPINTEGER	nExpResult = pItemElem.m_pPet->GetExp() * (EXPINTEGER)10000 / pItemElem.m_pPet->GetMaxExp();
				float fExp = (float)nExpResult / 100.0f;

				if (fExp >= 99.99f)
					strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_EXP_MAX));
				else
					strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_EXP), fExp);
				pEdit.AddString("\n");
				pEdit.AddString(strTemp, D3DCOLOR_XRGB(120, 120, 220));

				//Pet Energy
				int nMaxEnergy = pItemElem.m_pPet->GetMaxEnergy();
				int nEnergy = pItemElem.m_pPet->GetEnergy();
				int nLife = pItemElem.m_pPet->GetLife();
				pEdit.AddString("\n");
				strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_LIFE), nLife);
				pEdit.AddString(strTemp, D3DCOLOR_XRGB(255, 100, 100));
				pEdit.AddString("\n");
				strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_HP), nEnergy, nMaxEnergy);
				pEdit.AddString(strTemp, D3DCOLOR_XRGB(255, 10, 10));
			} else {
				//Pet Experience
				EXPINTEGER	nExpResult = pItemElem.m_pPet->GetExp() * (EXPINTEGER)10000 / pItemElem.m_pPet->GetMaxExp();
				float fExp = (float)nExpResult / 100.0f;

				if (fExp >= 99.99f)
					strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_EXP_MAX));
				else
					strTemp.Format(prj.GetText(TID_GAME_PET_TOOLTIP_EXP), fExp);
				pEdit.AddString("\n");
				pEdit.AddString(strTemp, D3DCOLOR_XRGB(120, 120, 220));
			}

			//Description
			strTemp.Format("%s", pItemElem.GetProp()->szCommand);
			pEdit.AddString("\n");
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwPiercing);
		}
	}

	void CTooltipBuilder::PutPetFeedPocket(
		const CItemElem & pItemElem, const ItemProp & itemProp,
		CEditString & pEdit
	) const {
		pEdit.Empty();

		//Name
		pEdit.AddString(itemProp.szName, dwItemColor[g_Option.m_nToolTipText].dwName0, ESSTY_BOLD);

		if (pItemElem.m_dwKeepTime <= 0) {
			// negation of ( 유료아이템이 사용된 상태인가? )
			
			//Description
			pEdit.AddString("\n");
			pEdit.AddString(prj.GetText(TID_GAME_PET_FEEDPOCKET_USE), dwItemColor[g_Option.m_nToolTipText].dwPiercing);
			return;
		}

		const bool hasFeedPocketBuff = g_pPlayer->HasBuff(BUFF_ITEM, II_SYS_SYS_SCR_PET_FEED_POCKET);
		
		const DWORD pocketUsingTooltipId = hasFeedPocketBuff ? TID_GAME_POCKETUSING : TID_GAME_POCKET_NOTUSING;

		CString strTemp;
		strTemp.Format(" %s", prj.GetText(pocketUsingTooltipId));
		pEdit.AddString(strTemp, D3DCOLOR_XRGB(255, 0, 0));

		//사용 제한 시한
		const DWORD timePeriodColor = hasFeedPocketBuff ? D3DCOLOR_XRGB(255, 20, 20) : dwItemColor[g_Option.m_nToolTipText].dwTime;
		
		time_t t = pItemElem.m_dwKeepTime - time_null();
		if (pItemElem.m_dwKeepTime && !pItemElem.IsFlag(CItemElem::expired)) {
			CString str = TimeAsStrWithOneNumberPrecision(t);
			strTemp = str + prj.GetText(TID_TOOLTIP_PERIOD);
			pEdit.AddString("\n");
			pEdit.AddString(strTemp, timePeriodColor);
		}

		if (hasFeedPocketBuff) {
			//사료 개수
			pEdit.AddString("\n");
			strTemp.Format("%s %d", prj.GetText(TID_GAME_PET_FEED_COUNT), g_pPlayer->GetItemNumForClient(II_SYS_SYS_FEED_01));
			pEdit.AddString(strTemp, D3DCOLOR_XRGB(50, 50, 205));
		}

		//Description
		strTemp.Format("%s", prj.GetText(TID_GAME_PET_FEEDPOCKET));
		pEdit.AddString("\n");
		pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwPiercing);
	}

	void CTooltipBuilder::PutNeededVis(const ItemProp & pItemPropVis, CEditString & pEdit) const {
		//gmpbigsun: 해당 비스를 착용하기 위해 필요한 비스 정보 출력 
		DWORD dwNeeds[2] = { pItemPropVis.dwReferTarget1, pItemPropVis.dwReferTarget2 };

		CString strTemp;
		DWORD color = 0xffffffff;

		NeedVis byState;
		const CItemElem * pPetItem = g_pPlayer->GetVisPetItem();
		if (!pPetItem) {
			//버프펫이 활성화가 안되어있따면, 필요비스 걍 빨간색으로 출력 
			byState = NeedVis::FailedBoth;
		} else {
			byState = CMover::IsSatisfyNeedVis(*pPetItem, pItemPropVis);
		}

		if (NULL_ID != dwNeeds[0] && 0 != dwNeeds[0]) {
			const ItemProp * pProp = prj.GetItemProp(dwNeeds[0]);		//sun!!
			strTemp.Format("\n%s: %s", GETTEXT(TID_GAME_BUFFPET_REQUIRE), pProp->szName); //필요비스

			color = 0xff000000;
			if (NeedVis::FailedBoth == byState || NeedVis::Failed1st == byState)
				color = 0xffff0000;

			pEdit.AddString(strTemp, color);
		}

		if (NULL_ID != dwNeeds[1] && 0 != dwNeeds[1]) {
			const ItemProp * pProp = prj.GetItemProp(dwNeeds[1]);
			strTemp.Format("\n%s: %s", GETTEXT(TID_GAME_BUFFPET_REQUIRE), pProp->szName);

			color = 0xff000000;
			if (NeedVis::FailedBoth == byState || NeedVis::Failed2nd == byState)
				color = 0xffff0000;

			pEdit.AddString(strTemp, color);
		}
	}

	void CTooltipBuilder::PutVisPetInfo(const CItemElem & pItemElem, CEditString & pEdit) const {
		//펫의 정보 (비스들의 총합 )
		std::map< int, int > cTotalOpt;
		CString strTemp2;

		int availableSlot = pItemElem.GetPiercingSize();

		// 장착된 모든 비스의 능력치 더하기 
		for (int ia = 0; ia < availableSlot; ++ia) {
			DWORD index = pItemElem.GetPiercingItem(ia);
			const ItemProp * pProp = prj.GetItemProp(index);
			if (!pProp) continue;

			if (time_null() >= pItemElem.GetVisKeepTime(ia)) continue;

			NeedVis bOK = CMover::IsSatisfyNeedVis(pItemElem, *pProp);

			if (NeedVis::Success == bOK)		//OK 활성중임.
			{
				for (int iaa = 0; iaa < ItemProp::NB_PROPS; ++iaa) {
					int nDst = (int)pProp->dwDestParam[iaa];
					if (NULL_ID == nDst)
						continue;

					int nVal = (int)pProp->nAdjParamVal[iaa];

					cTotalOpt[nDst] += nVal;
				}
			}
		}

		//전부 더해진 옵션에 대해서 출력 
		for (const auto & [nDst, nVal] : cTotalOpt) {
			const CString strTemp = SingleDstToString(SINGLE_DST{ nDst, nVal });
			pEdit.AddString(strTemp, dwItemColor[g_Option.m_nToolTipText].dwPiercing);
		}

		//장착된 비스 이름 + 남은 시간 출력 
		for (int ib = 0; ib < MAX_VIS; ++ib) {
			DWORD index = pItemElem.GetPiercingItem(ib);
			if (0 != index && NULL_ID != index) {
				ItemProp * pProp = prj.GetItemProp(index);
				if (!pProp)
					continue;

				DWORD dwKeepTime = pItemElem.GetVisKeepTime(ib);

				time_t t = dwKeepTime - time_null();
				CTimeSpan ct(t);

				//strTemp2.Format( prj.GetText( TID_TOOLTIP_DATE ), static_cast<int>(ct.GetDays()), ct.GetHours(), ct.GetMinutes(), ct.GetSeconds() );	// 지속시간 : 

			//	strTemp2.Format( prj.GetText( TID_TOOLTIP_DATE ), static_cast<int>(ct.GetDays()), ct.GetHours(), ct.GetMinutes(), ct.GetSeconds() );	// 지속시간 : 
				LONG nDay = (LONG)(ct.GetDays());
				LONG nHour = ct.GetHours();
				LONG nMin = ct.GetMinutes();
				LONG nSec = ct.GetSeconds();

				CString strDays, strHours, strMinutes, strSeconds;
				if (nDay > 0) {
					strDays.Format(prj.GetText(TID_PK_LIMIT_DAY), static_cast<int>(nDay));
					strHours.Format(prj.GetText(TID_PK_LIMIT_HOUR), nHour);
					strMinutes.Format(prj.GetText(TID_PK_LIMIT_MINUTE), nMin);

					strTemp2 = strDays + strHours + strMinutes;
				} else if (nHour > 0) {
					strHours.Format(prj.GetText(TID_PK_LIMIT_HOUR), nHour);
					strMinutes.Format(prj.GetText(TID_PK_LIMIT_MINUTE), nMin);

					strTemp2 = strHours + strMinutes;
				} else if (nMin > 0) {
					strMinutes.Format(prj.GetText(TID_PK_LIMIT_MINUTE), nMin);
					strSeconds.Format(prj.GetText(TID_PK_LIMIT_SECOND), nSec);

					strTemp2 = strMinutes + strSeconds;
				} else if (nSec >= 0) {
					strTemp2.Format(prj.GetText(TID_PK_LIMIT_SECOND), nSec);
				} else {
					strTemp2.Format(GETTEXT(TID_GAME_TOOLTIP_TIMEOUT)); //"시간 만료" 
				}

				CString strTemp;
				strTemp.Format("%s (%s)", pProp->szName, strTemp2.GetString());

				pEdit.AddString("\n");

				const DWORD color = t > 0
					? dwItemColor[g_Option.m_nToolTipText].dwTime
					: 0xffff0000;

				pEdit.AddString(strTemp, color);
			}
		}
	}

	void CTooltipBuilder::PutPetKind(const CItemElem & pItemElem, CEditString & pEdit) const {
		DWORD tooltipId = 0;

		if (pItemElem.IsEgg()) {
			tooltipId = TID_TOOLTIP_PET_REAR;
		} else if (pItemElem.IsEatPet()) {
			if (pItemElem.IsVisPet()) {
				tooltipId = TID_TOOLTIP_PET_BUFF;
			} else {
				tooltipId = TID_TOOLTIP_PET_PICKUP;
			}
		}

		if (tooltipId != 0) {
			CString strTemp;
			strTemp.Format("\n%s", prj.GetText(tooltipId));
			pEdit.AddString(strTemp);
		}
	}


	void CTooltipBuilder::PutSealChar(const CItemElem & pItemElem, CEditString & pEdit) const {
		
		CString str;
		str.Format("\n%s\n%s", prj.GetText(TID_TOOLTIP_CHARNAME), pItemElem.m_szItemText);
		CString strTemp = str;

		const int	nJob = pItemElem.m_nRepair;
		const int nLevel = static_cast<int>(pItemElem.m_nRepairNumber);

		CString strTemp2;
		if (MAX_PROFESSIONAL <= nJob && nJob < MAX_MASTER)
			strTemp2.Format("%d%s", nLevel, prj.GetText(TID_GAME_TOOLTIP_MARK_MASTER));
		else if (MAX_MASTER <= nJob)
			strTemp2.Format("%d%s", nLevel, prj.GetText(TID_GAME_TOOLTIP_MARK_HERO));
		else
			strTemp2.Format("%d", nLevel);

		str.Format("\n%s%s", prj.GetText(TID_TOOLTIP_CHARLEVEL), strTemp2);
		strTemp += str;

		CString strTemp3;
		strTemp3.Format("%s", prj.m_aJob[nJob].szName);

		str.Format("\n%s%s", prj.GetText(TID_TOOLTIP_CHARJOB), strTemp3);
		strTemp += str;
		str.Format("\n%s", prj.GetText(TID_TOOLTIP_CHARSTAT));
		strTemp += str;


		str.Format("\n%s%d", prj.GetText(TID_TOOLTIP_CHARSTA), (int)pItemElem.GetPiercingItem(0));
		strTemp += str;
		str.Format("\n%s%d", prj.GetText(TID_TOOLTIP_CHARSTR), (int)pItemElem.GetPiercingItem(1));
		strTemp += str;
		str.Format("\n%s%d", prj.GetText(TID_TOOLTIP_CHARDEX), (int)pItemElem.GetPiercingItem(2));
		strTemp += str;
		str.Format("\n%s%d", prj.GetText(TID_TOOLTIP_CHARINT), (int)pItemElem.GetPiercingItem(3));
		strTemp += str;
		str.Format("\n%s%d", prj.GetText(TID_TOOLTIP_CHARPOINT), (int)pItemElem.m_nResistAbilityOption);
		strTemp += str;

		pEdit.AddString(strTemp, D3DCOLOR_XRGB(255, 20, 20));
	}


	void CTooltipBuilder::PutEquipItemText(CEditString & pEdit) const {
		CString strEdit;
		strEdit.Format("[ %s ]\n", prj.GetText(TID_GAME_EQUIPED_ITEM));
		pEdit.AddString(strEdit, 0xff000000, ESSTY_BOLD);
	}


	CString CTooltipBuilder::TimeAsStrWithOneNumberPrecision(const time_t t) {
		CString str;

		if (t > 0) {
			CTimeSpan time(t);
			if (time.GetDays()) {
				str.Format(prj.GetText(TID_PK_LIMIT_DAY), static_cast<int>(time.GetDays() + 1));
			} else if (time.GetHours()) {
				str.Format(prj.GetText(TID_PK_LIMIT_HOUR), time.GetHours());
			} else if (time.GetMinutes()) {
				str.Format(prj.GetText(TID_PK_LIMIT_MINUTE), time.GetMinutes());
			} else {
				str.Format(prj.GetText(TID_PK_LIMIT_SECOND), time.GetSeconds());
			}
		}

		return str;
	}
}
