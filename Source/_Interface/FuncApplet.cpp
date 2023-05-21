#include "stdafx.h"
#include "defineText.h"
#include "AppDefine.h"
#include "WndCharacter.h"
#include "WndParty.h"
#include "WndNotice.h"
#include "WndMessenger.h"
#include "WndBank.h"		 // ���� / â��
#include "WndRepairItem.h"	 // ����â
#include "WndDebugInfo.h"    // ����� â 
#include "WndGuild.h"
#include "WndHelp.h"
#include "WndWebBox.h"
#include "WndVendor.h"
#include "WndCommItem.h"
#include "WndQuest.h"
#include "WndMotion.h"
#include "WndGuildVote.h"
#ifdef __IMPROVE_MAP_SYSTEM
#include "WndMapEx.h"
#else // __IMPROVE_MAP_SYSTEM
#include "WndMap.h"
#endif // __IMPROVE_MAP_SYSTEM
#include "WndBagEx.h"

#include "WndHousing.h"

#include "WndGuildHouse.h"
#include "WndBuffStatus.h"
#include "Wnd2ndPassword.h"
#include "WndEquipmentSex.h"
#include "WndSkillTree.h"
#include "WndSphereGrid.h"

#ifdef __NEW_WEB_BOX
#include "WndHelperWebBox.h"
#endif // __NEW_WEB_BOX
/*
     ���⼭ ���ǵ� ���÷� Ŭ������ �׽�ũ �Ŵ��� �߰��� �� �ִ� ������ �����Ѵ�.
	 �½�ũ �Ŵ��� ���÷��� �߰��ϱ� ���ؼ��� 

	 Step. 1 - Applet ID ����
	   ResData.h�� ���� Applet ID�� �����Ѵ�. �������μ����� Applet ID�� Applet�� �ν��Ѵ�.
	   ���� Daisy�� ����� ����� Daisy���� Id�� �������ָ� �ȴ�. �̰�� ���� ResData.h�� ������ �ʿ�� ����.

	 Step. 2 - DECLAREAPPLET ������ �Լ� �����ϱ�
	   DECLAREAPPLET�� ���� ����� �������� �� �ش� class�� �Ҵ��Ͽ� ���� ���μ������� �����͸�
	   �����ִ� ������ �Ѵ�. �� �Լ��� ���ǵǾ����� ������ ���� ���μ����� class�� ���� ��ų �� ���� �ȴ�.
	   DECLAREAPPLET�� AppMain �Լ��� ����� ���� �����ϰ� ������ define�̴�. ����� ������ ����.
	   DECLAREAPPLET( �Լ���, new className ); 

	 Step. 3 - map�� Add�ϱ�. ������ �ʵ��� ���� ������ Resource\textClient.inc�� ����. 
*/

template <typename T> CWndNeuz * WindowBuilder()
requires std::derived_from<T, CWndNeuz> {
	return new T();
}

void CWndMgr::AddAllApplet() {
	//             ������                    ID                            Ÿ��Ʋ                    ������                          ���� �ؽ�Ʈ 
	AddAppletFunc(WindowBuilder<CWndNavigator>    , APP_NAVIGATOR               , _T( "WndNavigator" )    , _T( "Icon_Navigator.dds" )    , GETTEXT( TID_TIP_NAVIGATOR      ),  'N'  );
	AddAppletFunc(WindowBuilder<CWndStatus>       , APP_STATUS1                 , _T( "WndStatus" )       , _T( "Icon_Status.dds"    )    , GETTEXT( TID_TIP_STATUS         ), 'T' );
#ifdef __IMPROVE_MAP_SYSTEM
	AddAppletFunc(WindowBuilder<CWndMapEx>        , APP_MAP_EX              , _T( "WndMap" )       , _T( "Icon_Applet.dds"    )    , GETTEXT(TID_TIP_MAP), 'M' );
#else // __IMPROVE_MAP_SYSTEM
	AddAppletFunc(WindowBuilder<CWndMap>          , APP_MAP                 , _T( "WndMap" )       , _T( "Icon_Applet.dds"    )    , GETTEXT(TID_TIP_MAP), 'M' );
#endif // __IMPROVE_MAP_SYSTEM
	
	const char lordSkillPosition = true ? 'A' : 'L';
	AddAppletFunc(WindowBuilder<CWndLordSkill>    , APP_LORD_SKILL             , _T( "WndLordSkill" )       , _T( "Icon_Infopang.dds"    )    , GETTEXT(TID_TIP_INFOPANG), lordSkillPosition);

	AddAppletFunc(WindowBuilder<CWndInfoPang>     , APP_INFOPANG                , _T( "WndInfoPang" )       , _T( "Icon_Infopang.dds"    )    , GETTEXT(TID_TIP_INFOPANG), 0 );
	AddAppletFunc(WindowBuilder<CWndHousing>      , APP_HOUSING                , _T( "WndHousing" )       , _T( "Icon_Housing.dds"    )    , GETTEXT(TID_GAME_HOUSING_BOX), 'Y' );

	AddAppletFunc(WindowBuilder<CWndGuildHousing> , APP_GH_FURNITURE_STORAGE, _T( "WndGuildHousing" )     , _T( "Icon_Housing.dds"    )    , GETTEXT(TID_GAME_HOUSING_BOX), 0 );
	AddAppletFunc(WindowBuilder<CWndCharacter>    , APP_CHARACTER3               , _T( "WndCharacter" )    , _T( "Icon_Character.dds" )    , GETTEXT( TID_TIP_CHARACTER      ), 'H' );
	AddAppletFunc(WindowBuilder<CWndInventory>    , APP_INVENTORY               , _T( "WndInventory" )    , _T( "Icon_Inventory.dds" )    , GETTEXT( TID_TIP_INVENTORY      ), 'I' );
	
	AddAppletFunc(WindowBuilder<CWndWebBox>       , APP_WEBBOX                  , _T( "WebBox" )          , _T( "Icon_CitemMall.dds" )    , GETTEXT( TID_TIP_ITEMMALL       ), 0 );
	
	if (APP_SKILL_ == APP_SKILL3) {
		AddAppletFunc(WindowBuilder<CWndSkillTreeEx>, APP_SKILL3, _T("WndSkill"), _T("Icon_Skill.dds"), GETTEXT(TID_TIP_SKILL), 'K');
	} else if (APP_SKILL_ == APP_SKILL_V16) {
		AddAppletFunc(WindowBuilder<CWndSkill_16>, APP_SKILL_V16, _T("WndSkill"), _T("Icon_Skill.dds"), GETTEXT(TID_TIP_SKILL), 'K');
	}
	

	AddAppletFunc(WindowBuilder<CWndMotion>       , APP_MOTION                  , _T( "WndMotion"    )    , _T( "Icon_Motion.dds"   )     , GETTEXT( TID_TIP_MOTION         ),  'O'  );
	AddAppletFunc(WindowBuilder<CWndTrade>        , APP_TRADE                   , _T( "WndTrade"     )    , _T( "Icon_Trade.dds"     )    , GETTEXT( TID_TIP_TRADE          ),  0  );

	AddAppletFunc(WindowBuilder<CWndVendor>	      , APP_VENDOR_REVISION         , _T( "WndVendor" )	   , _T( "Icon_Applet.dds" )       , GETTEXT( TID_TIP_VENDOR ), 0 );

	AddAppletFunc(WindowBuilder<CWndQuest>        , APP_QUEST_EX_LIST           , _T( "WndQuest"     )    , _T( "Icon_Quest.dds"     )    , GETTEXT( TID_TIP_QUEST          ), g_Neuz.Key.chQuest );
	AddAppletFunc(WindowBuilder<CWndParty>        , APP_PARTY                   , _T( "WndParty"     )    , _T( "Icon_Troupe.dds"    )    , GETTEXT( TID_TIP_PARTY          ),  'P' );

	AddAppletFunc(WindowBuilder<CWndGuild>        , APP_GUILD                   , _T( "WndGuild"     )    , _T( "Icon_Troupe.dds"    )    , GETTEXT( TID_TIP_COMPANY          ),  'G' );

#ifdef __GUILDVOTE
	AddAppletFunc(WindowBuilder<CWndGuildVote>    , APP_GUILD_VOTE              , _T( "WndGuildVote")     , _T( "Icon_Troupe.dds"    )    , GETTEXT( TID_TIP_COMPANY          ),  'V' );
#endif
	
	#ifndef __TMP_POCKET
	AddAppletFunc(WindowBuilder<CWndBagEx>        , APP_BAG_EX                , _T( "WndBagEx" )       , _T( "Icon_BagBag.tga"    )    , GETTEXT(TID_APP_BAG_EX), 'B' );
	#endif
	AddAppletFunc(WindowBuilder<CWndCommItem>     , APP_COMM_ITEM                , _T( "WndCommItem"  )    , _T( "Icon_CItemTime.dds" )    , GETTEXT( TID_TIP_ITEMTIME   ),  'J' );
	AddAppletFunc(WindowBuilder<CWndUpgradeBase>  , APP_TEST                  , _T( "WndUpgradeBase" )  , _T( "Icon_Troupe.dds"    )    , GETTEXT( TID_TIP_PARTY          ),  0 );
	AddAppletFunc(WindowBuilder<CWndPiercing>     , APP_PIERCING                 , _T( "WndPiercing"     ) , _T( "Icon_Troupe.dds"    )    , GETTEXT( TID_TIP_PARTY          ),  0 );
	AddAppletFunc(WindowBuilder<CWndChat>         , APP_COMMUNICATION_CHAT      , _T( "WndChat"      )    , _T( "Icon_Chat.dds"      )    , GETTEXT( TID_TIP_COMMUNICATION_CHAT    ),  0 );
	AddAppletFunc(WindowBuilder<CWndMessengerEx>  , APP_MESSENGER_              , _T( "WndMessenger" )    , _T( "Icon_Messenger.dds"   )  , GETTEXT( TID_TIP_MESSENGER ),  'E'  );

	AddAppletFunc(WindowBuilder<CWndOptSound>     , APP_OPTION_SOUND            , _T( "WndOptSound" )     , _T( "Icon_OptSound.dds" )     , GETTEXT( TID_TIP_OPTION_SOUND          ),  0  );
	AddAppletFunc(WindowBuilder<CWndOptWindow>    , APP_OPTION_WINDOW           , _T( "WndOptWindow" )    , _T( "Icon_Applet.dds"      )  , GETTEXT( TID_TIP_OPTION_WINDOW         ),  0  );
	AddAppletFunc(WindowBuilder<CWndOptMyInfo>    , APP_OPTION_MYINFO           , _T( "WndOptMyInfo" )    , _T( "Icon_Applet.dds" )       , GETTEXT( TID_TIP_OPTION_MYINFO         ),  0  );
	AddAppletFunc(WindowBuilder<CWndTotalOption>  , APP_OPTIONEX				 , _T( "WndOption" )	   , _T( "Icon_Applet.dds" )	   , GETTEXT( TID_APP_OPTION				),	0  );

	AddAppletFunc(WindowBuilder<CWndInfoNotice>   , APP_INFO_NOTICE             , _T( "WndInfoNotice" )   , _T( "Icon_Applet.dds" )       , GETTEXT( TID_TIP_INFO_NOTICE           ),  0  );
	AddAppletFunc(WindowBuilder<CWndHelp>         , APP_HELPER_HELP             , _T( "WndHelpHelp" )     , _T( "Icon_HelperHelp.dds" )   , GETTEXT( TID_TIP_HELPER_HELP           ),  0  );
	AddAppletFunc(WindowBuilder<CWndHelpTip>      , APP_HELPER_TIP              , _T( "WndHelpTip" )      , _T( "Icon_HelperTip.dds" )    , GETTEXT( TID_TIP_HELPER_TIP            ),  0  );
	AddAppletFunc(WindowBuilder<CWndHelpFAQ>      , APP_HELPER_FAQ              , _T( "WndHelpFAQ" )      , _T( "Icon_HelperFAQ.dds" )    , GETTEXT( TID_TIP_HELPER_FAQ            ),  0  );

	AddAppletFunc(WindowBuilder<CWndLogOut>       , APP_LOGOUT                  , _T( "WndLogout"   )     , _T( "Icon_Logout.dds"   )     , GETTEXT( TID_TIP_LOGOUT              ),  0  );
	AddAppletFunc(WindowBuilder<CWndQuit>         , APP_QUIT                    , _T( "WndQuit"      )    , _T( "Icon_Quit.dds"      )    , GETTEXT( TID_TIP_QUIT                  ),  0  );
	AddAppletFunc(WindowBuilder<CWndWebBox2>      , APP_WEBBOX2                 , _T( "WebBox2" )         , _T( "QOODO.dds" )             , GETTEXT( TID_TIP_QOODO	    ), 0 );

	AddAppletFunc(WindowBuilder<CWndWorld>        , APP_WORLD                   , _T( "WndWorld" )        , _T( "Icon_Applet.dds" )       , GETTEXT( TID_TIP_WORLD     ),  0  );
	AddAppletFunc(WindowBuilder<CWndDebugInfo>    , APP_DEBUGINFO               , _T( "WndDebugInfo" )    , _T( "Icon_Applet.dds" )       , GETTEXT( TID_TIP_DEBUGINFO ),  0  );
	AddAppletFunc(WindowBuilder<CWndLogin>        , APP_LOGIN                   , _T( "WndLogin"      )   , _T( "Icon_Login.dds"      )   , GETTEXT( TID_TIP_APPLET    ),  0  );
	AddAppletFunc(WindowBuilder<CWndSelectServer> , APP_SELECT_SERVER           , _T( "WndSelectServer")  , _T( "Icon_SelectServer.dds")  , GETTEXT( TID_TIP_WORLD     ),  0  );
	AddAppletFunc(WindowBuilder<CWndCreateChar>   , APP_CREATE_CHAR             , _T( "WndCreateChar" )   , _T( "Icon_CreateChar.dds" )   , GETTEXT( TID_TIP_WORLD     ),  0  );
	AddAppletFunc(WindowBuilder<CWndSelectChar>   , APP_SELECT_CHAR             , _T( "WndSelectChar" )   , _T( "Icon_SelectChar.dds" )   , GETTEXT( TID_TIP_DIALOG    ),  0  );

	AddAppletFunc(WindowBuilder<CWndPartyQuick>   , APP_PARTY_QUICK             , _T( "WndPartyQuick" )   , NULL   , GETTEXT( TID_TIP_DIALOG    ),  0  );
	AddAppletFunc(WindowBuilder<CWndBuffStatus>   , APP_BUFF_STATUS             , _T( "WndBuffStatus" )   , NULL   , GETTEXT( TID_TIP_DIALOG    ),  0  );

	const char theFrenchAreBack = true ? 'X' : 'F';
	AddAppletFunc(WindowBuilder<CWndCoupleManager>, APP_COUPLE_MAIN             , _T( "WndCoupleManager" )   , _T( "Icon_Couple.dds" )   , GETTEXT( TID_GAME_COUPLE ), theFrenchAreBack);

	AddAppletFunc(WindowBuilder<CWnd2ndPassword>  , APP_2ND_PASSWORD_NUMBERPAD  , _T( "Wnd2ndPassword" )   , _T( "Icon_Applet.dds" )   , GETTEXT( TID_2ND_PASSWORD_WINDOW_OPEN ),  0  );
#ifdef __NEW_WEB_BOX
	AddAppletFunc(WindowBuilder<CWndHelperWebBox> , APP_WEBBOX2              , _T( "HelperWebBox" )       , _T( "Icon_HelperHelp.dds" ) , GETTEXT( TID_GAME_HELPER_WEB_BOX_ICON_TOOLTIP ), 0 );
#endif // __NEW_WEB_BOX

	AddAppletFunc(WindowBuilder<CWndEquipmentSex>, APP_EQUIPMENT_SEX, _T("EquipmentSex"), _T("Icon_Applet.dds"), "Equipement Sex", 0);
	AddAppletFunc(WindowBuilder<CWndSphereGrid>, APP_SPHEREGRID, _T("RikkuIsBestGirl"), _T("Icon_Applet.dds"), "Sphere Grid", 'R');
}
