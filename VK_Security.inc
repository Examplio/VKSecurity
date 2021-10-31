/*

Description: VK Security System
Author: Examplio
MY VK: ( vk.com/example_text )
Copyright (c) 2021 Viktor Kasko

*/

#if defined _vk_security_included
    #endinput
#endif
#define _vk_security_included

#tryinclude <a_samp>

#if !defined DOMAIN
    #error Please define your domain before including file. Example: #define DOMAIN "apple.com"
#endif

#if defined V_VERSION
    #undef V_VERSION
#endif
#define V_VERSION   (1) // VK SECURITY VERSION

#if !defined _vk_security_lang_included
    #tryinclude <_vk_security_en.lang>

    #if !defined _vk_security_lang_included
        #tryinclude <_vk_security_ru.lang>

        #if !defined _vk_security_lang_included
            #error Didn`t find language pack for VK Security. Check file _vk_security_ru.lang or _vk_security_en.lang
        #endif
    #endif
#endif

#if (VK_LANG_VERSION != V_VERSION)
    #error Update or install language pack(script) for current version to use VK SECURITY correctly. Check files _vk_security_ru.lang or _vk_security_en.lang (VK_Security.inc)
#endif

#if !defined HTTP

    #tryinclude <a_http>

    #if !defined HTTP
        native HTTP(index, type, url[], data[], callback[]);
    #endif
#endif

#if __Pawn >= 0x3A && defined __PawnBuild
    #if __Pawn > 0x3A || __PawnBuild >= 2
        #define static_enum        static enum
    #endif
#endif

#if !defined static_enum
    #define static_enum            enum
#endif

#if !defined VK_COLOR
    #define VK_COLOR    "{0099CC}"
#endif


static
    static_VkActivated[MAX_PLAYERS char] = {0, ...},
    static_VK_ID[MAX_PLAYERS] = {0, ...},
    static_Timer[MAX_PLAYERS],
    static_name[MAX_PLAYERS][MAX_PLAYER_NAME],
    static_VkType[MAX_PLAYERS];

static_enum {
    D_MAIN_PANEL,
    D_VK_CONFIRMATION,
    D_VK_CONFIRMATION_1,
    D_VK_CONFIRMATION_DECLINE,
    D_VK_AUTH_WINDOW
};

// Stocks

stock VK_ShowMain(playerid) {

    static
        message_string[sizeof MAIN_PANEL_MSG + sizeof T_OPERATOR_TYPE_2];
    
    format(message_string,sizeof(message_string),MAIN_PANEL_MSG,static_VkType [ playerid ] == 0 ? T_OPERATOR_TYPE_1 : T_OPERATOR_TYPE_2);

    return ShowPlayerDialog(playerid,D_MAIN_PANEL,DIALOG_STYLE_LIST,DIALOG_TITLE,message_string,BUTTON_YES,BUTTON_NO ) ;
}

stock VK_DEL_SESSION(playerid) {

    static
        http_string[255 + 35 /* 255 - max domain length*/];

    format(http_string, sizeof(http_string), "%s/_handler.php?to=%i&pid=-1",DOMAIN, static_VK_ID [ playerid ]);

    return HTTP(playerid, 2, http_string, "", "");
}

stock VK_ShowConfirmation( playerid ) {
    if( static_VkActivated { playerid } )
        return 1;
    
    return ShowPlayerDialog(playerid,D_VK_CONFIRMATION,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,CONFIRMATION_MSGBOX,BUTTON_YES,BUTTON_NO);
}

stock VK_ShowAuthWindow ( playerid ) {
    if( !static_VkActivated { playerid } )
        return true;

    static
        http_string[255 + 35 /* 255 - max domain length*/];

    format(http_string, sizeof(http_string), "%s/_handler.php?name=%s&to=%d",DOMAIN,static_name [ playerid ], static_VK_ID [ playerid ]);

    HTTP(playerid, 2, http_string, "", "_Info");
    
    ShowPlayerDialog(playerid,D_VK_AUTH_WINDOW,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,D_VK_WINDOW,SPECIAL_BUTTON,!"");

    return 1;
}
// some publics 

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) {

    switch(dialogid) {
        case D_VK_AUTH_WINDOW: {
            return ShowPlayerDialog(playerid,D_VK_AUTH_WINDOW,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,D_VK_WINDOW,SPECIAL_BUTTON,!"");
        }
        case D_MAIN_PANEL: {
            if(!response)
                return true;
            
            switch(listitem) {
                case 0: return VK_ShowMain(playerid);
                case 1: {
                    static_VkType [ playerid ] = !static_VkType [ playerid ];
                    return VK_ShowMain(playerid);
                }
                case 2: {
                    if(!static_VkActivated { playerid })
                        return 1;
                    
                    static 
                        message_string[sizeof CONFIRMATION_DECLINE + 6],
                        rand;

                    rand = random(999999-100000)+100000;

                    format(message_string,sizeof(message_string),CONFIRMATION_DECLINE,rand);

                    static const qString[] = "\
                        UPDATE `vk_security` SET `u_code` = '%i' WHERE `u_name` = '%s'"
                    ;

                    static
                        query_string[sizeof qString + MAX_PLAYER_NAME + 6];

                    format(query_string,sizeof(query_string),qString,rand, static_name [ playerid ] );

                    #if !defined mysql_reconnect
                        mysql_tquery(MYSQL_DEFAULT_HANDLE,query_string,"","");
                    #else
                        #if defined cache_get_row_int
                            mysql_tquery(1,query_string,"","");
                        #endif
                    #endif

                    dialog_decline:

                    return ShowPlayerDialog(playerid,D_VK_CONFIRMATION_DECLINE,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,message_string,BUTTON_YES,BUTTON_NO);
                }
            }
        }
        case D_VK_CONFIRMATION_DECLINE: {
            if(!response)
            {
                static const qString[] = "\
                    UPDATE `vk_security` SET `u_code` = '0' WHERE `u_name` = '%s'"
                ;

                static
                    query_string[sizeof qString + MAX_PLAYER_NAME];

                format(query_string,sizeof(query_string),qString, static_name [ playerid ] );

                #if !defined mysql_reconnect
                    mysql_tquery(MYSQL_DEFAULT_HANDLE,query_string,"","");
                #else
                    #if defined cache_get_row_int
                        mysql_tquery(1,query_string,"","");
                    #endif
                #endif

                return SendClientMessage(playerid,-1,DECLINE_SCM);
            }

            static const qString[] = "\
                SELECT `u_userid`,`u_code` FROM `vk_security` WHERE `u_name` = '%s'"
            ;

            static
                query_string[sizeof qString + MAX_PLAYER_NAME];

            format(query_string,sizeof(query_string),qString, static_name [ playerid ] );

            #if !defined mysql_reconnect
                new
                    Cache:qHandle = mysql_query(MYSQL_DEFAULT_HANDLE,query_string);

                static
                    _uID;
                
                cache_get_value_name_int(0,!"u_userid",_uID);

                if(!_uID)
                {
                    cache_delete(qHandle);

                    static_VkActivated { playerid } = 0;
                    static_VK_ID [ playerid ] = 0;
                    static_VkType [ playerid ] = 0;

                    return SendClientMessage(playerid,-1,UNLINK);
                } else {
                    cache_delete(qHandle);
                    goto dialog_decline;
                    return 1;
                }
            #else
                #if defined cache_get_row_int
                    new
                        Cache:qHandle = mysql_query(1,query_string);

                    static
                        _uID;

                    _uID = cache_get_field_content_int(0,!"u_userid");

                    if(!_uID)
                    {
                        cache_delete(qHandle);

                        static_VkActivated { playerid } = 0;
                        static_VK_ID [ playerid ] = 0;
                        static_VkType [ playerid ] = 0;

                        return SendClientMessage(playerid,-1,UNLINK);
                    } else {
                        cache_delete(qHandle);
                        goto dialog_decline;
                        return 1;
                    }
                #endif
            #endif
        }
        case D_VK_CONFIRMATION: {
            if(!response)
                return true;

            static 
                message_string[sizeof CONFIRMATION_INPUT + 6],
                rand;

            rand = random(999999-100000)+100000;

            format(message_string,sizeof(message_string),CONFIRMATION_INPUT,rand);

            static const qString[] = "\
                UPDATE `vk_security` SET `u_code` = '%i' WHERE `u_name` = '%s'"
            ;

            static
                query_string[sizeof qString + MAX_PLAYER_NAME + 6];

            format(query_string,sizeof(query_string),qString,rand, static_name [ playerid ] );
            
            #if !defined mysql_reconnect
                mysql_tquery(MYSQL_DEFAULT_HANDLE,query_string,"","");
            #else
                #if defined cache_get_row_int
                    mysql_tquery(1,query_string,"","");
                #endif
            #endif

            d_dialog:
            
            return ShowPlayerDialog(playerid,D_VK_CONFIRMATION_1,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,message_string,BUTTON_YES,BUTTON_NO);
        }
        case D_VK_CONFIRMATION_1: {
            if(!response) {
                static const qString[] = "\
                    UPDATE `vk_security` SET `u_code` = '0',`u_userid` = '0' WHERE `u_name` = '%s'"
                ;

                static
                    query_string[sizeof qString + MAX_PLAYER_NAME];

                format(query_string,sizeof(query_string),qString, static_name [ playerid ] );

                #if !defined mysql_reconnect
                    mysql_tquery(MYSQL_DEFAULT_HANDLE,query_string,"","");
                #else
                    #if defined cache_get_row_int
                        mysql_tquery(1,query_string,"","");
                    #endif
                #endif

                return 1;
            }

            static const qString[] = "\
                    SELECT `u_userid`,`u_code` FROM `vk_security` WHERE `u_name` = '%s'"
                ;

            static
                query_string[sizeof qString + MAX_PLAYER_NAME];

            format(query_string,sizeof(query_string),qString, static_name [ playerid ] );

            #if !defined mysql_reconnect
                new
                    Cache:qHandle = mysql_query(MYSQL_DEFAULT_HANDLE,query_string);

                static
                    _uID;
                
                cache_get_value_name_int(0,!"u_userid",_uID);

                if(!_uID)
                {
                    cache_delete(qHandle);

                    goto d_dialog;

                    return 1;
                }
                
                static_VkActivated { playerid } = 1;
                static_VK_ID [ playerid ] = _uID;
                static_VkType [ playerid ] = 0;

            #else
                #if defined cache_get_row_int
                    new
                        Cache:qHandle = mysql_query(1,query_string);

                    static
                        _uID;

                    _uID = cache_get_field_content_int(0,!"u_userid");

                    if(!_uID)
                        return cache_delete(qHandle), goto return_dialog;
                
                    static_VkActivated { playerid } = 1;
                    static_VK_ID [ playerid ] = _uID;
                    static_VkType [ playerid ] = 0;
                #endif
            #endif

            SendClientMessage(playerid,-1,ACTIVATION_MSG);

            return cache_delete(qHandle);
        }
    }

    #if defined SECURITY_OnDialogResponse
        return SECURITY_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
    #endif
}

public OnGameModeInit()
{
    #if defined SECURITY_OnGameModeInit
        SECURITY_OnGameModeInit();
    #endif

    #if !defined mysql_reconnect
        mysql_query(MYSQL_DEFAULT_HANDLE, "CREATE TABLE IF NOT EXISTS `vk_security` (`u_name` varchar(24) CHARACTER SET cp1251 NOT NULL UNIQUE,`u_userid` int(11) NOT NULL DEFAULT '0',`u_code` int(11) NOT NULL DEFAULT '0',`u_type` int(11) NOT NULL DEFAULT '0') ENGINE=InnoDB DEFAULT CHARSET=cp1251;", false);
    #else
        #if defined cache_get_row_int
            mysql_query(1, "CREATE TABLE IF NOT EXISTS `vk_security` (`u_name` varchar(24) CHARACTER SET cp1251 NOT NULL UNIQUE,`u_userid` int(11) NOT NULL DEFAULT '0',`u_code` int(11) NOT NULL DEFAULT '0',`u_type` int(11) NOT NULL DEFAULT '0') ENGINE=InnoDB DEFAULT CHARSET=cp1251;", false);
        #endif
    #endif
    return 1;
}

public OnPlayerConnect(playerid)
{
    #if defined SECURITY_OnPlayerConnect
        SECURITY_OnPlayerConnect(playerid);
    #endif

    static
        query_string[50 + MAX_PLAYER_NAME];  

    GetPlayerName(playerid,  static_name [ playerid ] , MAX_PLAYER_NAME);

    format(query_string, sizeof(query_string), "SELECT * FROM `vk_security` WHERE `u_name` = '%s'",  static_name [ playerid ] );

    #if !defined mysql_reconnect
        new
            Cache:qHandle = mysql_query(MYSQL_DEFAULT_HANDLE,query_string);

        if(cache_num_rows()) {
            cache_get_value_name_int(0,!"u_type",static_VkType [ playerid ]);
            cache_get_value_name_int(0,!"u_userid",static_VK_ID [ playerid ]);
            
            if(static_VK_ID [ playerid ] != 0)
                static_VkActivated{playerid} = 1;
        } else {
            format(query_string,sizeof(query_string),"\
                INSERT INTO `vk_security` (`u_name`) VALUES ('%s')",
                 static_name [ playerid ] 
            );
            mysql_tquery(MYSQL_DEFAULT_HANDLE,query_string,"","");
        }
    #else
        #if defined cache_get_row_int
            new
                Cache:qHandle = mysql_query(1,query_string);

            if(cache_num_rows()) {
                static_VkType [ playerid ] = cache_get_field_content_int(0,!"u_type");
                static_VK_ID [ playerid ] = cache_get_field_content_int(0,!"u_userid");

                if(static_VK_ID [ playerid ] != 0)
                    static_VkActivated{playerid} = 1;
            } else {
                format(query_string,sizeof(query_string),"\
                    INSERT INTO `vk_security` (`u_name`) VALUES ('%s')",
                    static_name [ playerid ] 
                );
                mysql_tquery(1,query_string,"","");
            }
        #endif
    #endif

    return cache_delete(qHandle);            
}

public OnPlayerDisconnect(playerid, reason) {
    #if defined SECURITY_OnPlayerDisconnect
        SECURITY_OnPlayerDisconnect(playerid,reason);
    #endif

    static_VkType [ playerid ] = 0;
    static_VK_ID [ playerid ] = 0;
    static_VkActivated { playerid } = 0;
    KillTimer ( static_Timer [ playerid ] ) ;
    static_name [ playerid ] [ 0 ] = EOS;

    return 1;
}

// Forward`s
forward SECURITY_OnPlayerDisconnect(playerid,reason);
forward SECURITY_OnPlayerConnect(playerid);
forward SECURITY_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
forward SECURITY_OnGameModeInit();
//Timer and HTTP receiver
forward _RealTime_Info(playerid);
public _RealTime_Info(playerid) {

    static
        http_string[255 + 35 /* 255 - max domain length*/];

    format(http_string, sizeof(http_string), "%s/sessions/session%d.txt",DOMAIN, static_VK_ID [ playerid ]);

    HTTP(playerid, 1, http_string, "", "_Info_");
}
forward _Info_(index, response_code, data[]);
public _Info_(index, response_code, data[]) {

    if(response_code >= 200 && response_code <= 299 && !strcmp(data,"1")) {
        ShowPlayerDialog(index, -1, 0, " ", " ", " ", " ");
        SendClientMessage(index, -1, SUCESSFULLY_AUTH);
        VK_DEL_SESSION(index);
        return KillTimer( static_Timer [ index ] ) ;
    }
    else if(response_code >= 200 && response_code <= 299 && !strcmp(data,"-1")) {
        SendClientMessage(index, -1, BLOCKED_AUTH);
        KillTimer( static_Timer [ index ] ) ;
        VK_DEL_SESSION(index);
        return Kick(index);
    }
    return 1;
}
forward _Info(index, response_code, data[]);
public _Info(index, response_code, data[]) {

    if(response_code == 200 && !strcmp(data,"ok")) {
        static_Timer [ index ] = SetTimerEx("_RealTime_Info",700,1,"i",index);
    }
    else SendClientMessage(index,-1,IS_NOT_AVAILABLE);
}
// Other

#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif

#define OnPlayerDisconnect SECURITY_OnPlayerDisconnect

#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif

#define OnPlayerConnect SECURITY_OnPlayerConnect

#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif

#define OnGameModeInit SECURITY_OnGameModeInit


#if defined _ALS_OnDialogResponse
    #undef OnDialogResponse
#else
    #define _ALS_OnDialogResponse
#endif

#define OnDialogResponse SECURITY_OnDialogResponse