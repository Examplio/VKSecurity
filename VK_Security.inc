/**
 * Written by Examplio with ❤
 * VK: vk.com/vv.kasko
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
#define V_VERSION   (4)

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
        native HTTP(index, type, url[], data[], callback[] ) ;
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

#if !defined MAX_IP_LENGTH
    #define MAX_IP_LENGTH       16
#endif

// MySQL aspects

#if !defined mysql_reconnect
    static 
        MySQL:g_vk_security_mysql_handle;

    stock VK_SetMySQLHandle(MySQL:handle)
    {
        g_vk_security_mysql_handle = handle;
        return 1;
    }
#else
    #if defined cache_get_row_int
        static 
            g_vk_security_mysql_handle;

        stock VK_SetMySQLHandle(handle)
        {
            g_vk_security_mysql_handle = handle;
            return 1;
        }
    #endif
#endif

static
    static_VkActivated [ MAX_PLAYERS char ] = {0, ...},
    static_VK_ID [ MAX_PLAYERS ] = {0, ...},
    static_Timer [ MAX_PLAYERS ],
    static_name [ MAX_PLAYERS ] [ MAX_PLAYER_NAME ],
    static_VkType [ MAX_PLAYERS ] = {0, ...},
    static_isLogged [ MAX_PLAYERS char ] = {0 , ...},
    static_ip [ MAX_PLAYERS ] [ MAX_IP_LENGTH ];

static_enum {
    D_MAIN_PANEL = 20000,
    D_VK_CONFIRMATION,
    D_VK_CONFIRMATION_1,
    D_VK_CONFIRMATION_DECLINE,
    D_VK_AUTH_WINDOW
};

// Stocks

stock VK_IsConfirmed ( playerid ) {
    return static_VK_ID [ playerid ] ? 1 : 0;
}

stock VK_isLogged ( playerid ) {
    return (! static_isLogged { playerid } ) ? 0 : 1;
}

stock VK_SetNewName ( playerid , n_Name[]) {

    static const q_str[] = "\
        UPDATE `vk_security` SET `u_name` = '%s' WHERE `u_name` = '%s'"
    ;

    static
        vk_string [ sizeof q_str + (MAX_PLAYER_NAME * 2) + ( 1 + 1 ) ] ;

    format(vk_string , sizeof ( vk_string ) , q_str , n_Name , static_name [ playerid ] ) ;

    mysql_tquery ( g_vk_security_mysql_handle , vk_string , "" , "" ) ;

    format ( static_name [ playerid ] , sizeof static_name [ ] , n_Name  ) ;

    return 1;
}

stock VK_ShowMain ( playerid )  {

    if ( ! static_VkActivated { playerid } )
        return SendClientMessage ( playerid,-1,DONT_HAVE ) ;

    static
        message_string[sizeof MAIN_PANEL_MSG + sizeof T_OPERATOR_TYPE_2];
    
    format(message_string,sizeof(message_string),MAIN_PANEL_MSG, 0 == static_VkType [ playerid ] ? T_OPERATOR_TYPE_1 : T_OPERATOR_TYPE_2 ) ;

    ShowPlayerDialog ( playerid,D_MAIN_PANEL,DIALOG_STYLE_LIST,DIALOG_TITLE,message_string,BUTTON_YES,BUTTON_NO ) ;

    return 1;
}

stock VK_DEL_SESSION ( playerid )  {

    static
        http_string[255 + 35 /* 255 - max domain length*/];

    format(http_string, sizeof(http_string), "%s/client/receiver/_handler.php?to=%i&pid=-1",DOMAIN, static_VK_ID [ playerid ] ) ;

    HTTP ( playerid, 2, http_string, "", "" ) ;

    return 1;
}

stock VK_ShowConfirmation( playerid ) {

    if( static_VkActivated { playerid } )
        return SendClientMessage ( playerid , -1 , HAVE ) ;
    
    ShowPlayerDialog ( playerid,D_VK_CONFIRMATION,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,CONFIRMATION_MSGBOX,BUTTON_YES,BUTTON_NO ) ;

    return 1;
}

stock VK_ShowAuthWindow ( playerid ) {
    if( !static_VkActivated { playerid } )
        return SendClientMessage( playerid , -1 , DONT_HAVE ) ;

    static
        static_temp_ip [ MAX_IP_LENGTH ] ;

    GetPlayerIp ( playerid, static_temp_ip, MAX_IP_LENGTH ) ;

    if ( ! strcmp( static_temp_ip, static_ip [ playerid ] ) && static_VkType [ playerid ] == 1)
        return 0;

    static
        http_string[255 + 35 /* 255 - max domain length*/];

    format(http_string, sizeof(http_string), "%s/client/receiver/_handler.php?name=%s&to=%d",DOMAIN,static_name [ playerid ], static_VK_ID [ playerid ] ) ;

    HTTP ( playerid, 2, http_string, "", "_SendRequest" ) ;

    ShowPlayerDialog ( playerid,D_VK_AUTH_WINDOW,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,D_VK_WINDOW,SPECIAL_BUTTON,!"" ) ;

    return 1;
}

public OnDialogResponse ( playerid, dialogid, response, listitem, inputtext[]) {

    switch(dialogid) {
        case D_VK_AUTH_WINDOW: {
            return ShowPlayerDialog ( playerid,D_VK_AUTH_WINDOW,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,D_VK_WINDOW,SPECIAL_BUTTON,!"" ) ;
        }
        case D_MAIN_PANEL: {
            if (!response)
                return true;
            
            switch(listitem) {
                case 0: VK_ShowMain ( playerid ) ;
                case 1: {
                    static_VkType [ playerid ] = !static_VkType [ playerid ];
                    return VK_ShowMain ( playerid ) ;
                }
                case 2: {
                    if ( ! static_VkActivated { playerid })
                        return 1;
                    
                    static 
                        message_string[sizeof CONFIRMATION_DECLINE + 6],
                        rand;

                    rand = random(999999-100000)+100000;

                    format(message_string,sizeof(message_string),CONFIRMATION_DECLINE,rand ) ;

                    static const qString[] = "\
                        UPDATE `vk_security` SET `u_code` = '%i' WHERE `u_name` = '%s'"
                    ;

                    static
                        query_string[sizeof qString + MAX_PLAYER_NAME + 6];

                    format(query_string,sizeof(query_string),qString,rand, static_name [ playerid ]  ) ;

                    mysql_tquery ( g_vk_security_mysql_handle,query_string,"","" ) ;

                    dialog_decline:

                    return ShowPlayerDialog ( playerid,D_VK_CONFIRMATION_DECLINE,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,message_string,BUTTON_YES,BUTTON_NO ) ;
                }
            }
        }
        case D_VK_CONFIRMATION_DECLINE: {
            if ( ! response )
            {
                static const qString[] = "\
                    UPDATE `vk_security` SET `u_code` = '0' WHERE `u_name` = '%s'"
                ;

                static
                    query_string[sizeof qString + MAX_PLAYER_NAME];

                format(query_string,sizeof(query_string),qString, static_name [ playerid ]  ) ;

                mysql_tquery ( g_vk_security_mysql_handle,query_string,"","" ) ;

                return SendClientMessage ( playerid,-1,DECLINE_SCM ) ;
            }

            static const qString[] = "\
                SELECT `u_userid`,`u_code` FROM `vk_security` WHERE `u_name` = '%s'"
            ;

            static
                query_string[sizeof qString + MAX_PLAYER_NAME];

            format(query_string,sizeof(query_string),qString, static_name [ playerid ]  ) ;

            #if !defined mysql_reconnect
                new
                    Cache:qHandle = mysql_query ( g_vk_security_mysql_handle,query_string ) ;

                static
                    _uID;
                
                cache_get_value_name_int(0,!"u_userid",_uID ) ;
                
                cache_delete ( qHandle ) ;

                if ( ! _uID)
                {
                    static_VkActivated { playerid } = 0;

                    static_VK_ID [ playerid ] = 0;

                    static_VkType [ playerid ] = 0;

                    return SendClientMessage ( playerid,-1,UNLINK ) ;

                } 
                else {
                    goto dialog_decline;
                    
                    return 1;
                }

            #else
                #if defined cache_get_row_int
                    new
                        Cache:qHandle = mysql_query ( g_vk_security_mysql_handle,query_string ) ;

                    static
                        _uID;

                    _uID = cache_get_field_content_int(0,!"u_userid",g_vk_security_mysql_handle ) ;

                    cache_delete ( qHandle ) ;

                    if ( ! _uID)
                    {

                        static_VkActivated { playerid } = 0;
                        
                        static_VK_ID [ playerid ] = 0;

                        static_VkType [ playerid ] = 0;

                        return SendClientMessage ( playerid,-1,UNLINK ) ;

                    } 
                    else {
                        goto dialog_decline;

                        return 1;
                    }
                #endif
            #endif
        }
        case D_VK_CONFIRMATION: {

            if (!response)
                return true;

            static 
                message_string[sizeof CONFIRMATION_INPUT + 6],
                rand;

            rand = random(999999-100000)+100000;

            format(message_string,sizeof(message_string),CONFIRMATION_INPUT,rand ) ;

            static const qString[] = "\
                UPDATE `vk_security` SET `u_code` = '%i' WHERE `u_name` = '%s'"
            ;

            static
                query_string[sizeof qString + MAX_PLAYER_NAME + 6];

            format(query_string,sizeof(query_string),qString,rand, static_name [ playerid ]  ) ;
            
            mysql_tquery ( g_vk_security_mysql_handle , query_string , "" , "" ) ;

            d_dialog:
            
            ShowPlayerDialog ( playerid,D_VK_CONFIRMATION_1,DIALOG_STYLE_MSGBOX,DIALOG_TITLE,message_string,BUTTON_YES,BUTTON_NO ) ;
        }
        case D_VK_CONFIRMATION_1: {

            if (!response) {
                static const qString[] = "\
                    UPDATE `vk_security` SET `u_code` = '0',`u_userid` = '0' WHERE `u_name` = '%s'"
                ;

                static
                    query_string[sizeof qString + MAX_PLAYER_NAME];

                format(query_string,sizeof(query_string),qString, static_name [ playerid ]  ) ;

                mysql_tquery ( g_vk_security_mysql_handle,query_string,"","" ) ;

                return 1;
            }

            static const qString[] = "\
                    SELECT `u_userid`,`u_code` FROM `vk_security` WHERE `u_name` = '%s'"
                ;

            static
                query_string[sizeof qString + MAX_PLAYER_NAME];

            format(query_string,sizeof(query_string),qString, static_name [ playerid ]  ) ;

            #if !defined mysql_reconnect
                new
                    Cache:qHandle = mysql_query ( g_vk_security_mysql_handle,query_string ) ;

                static
                    _uID;
                
                cache_get_value_name_int(0,!"u_userid",_uID ) ;

                if ( ! _uID)
                {
                    cache_delete ( qHandle ) ;

                    goto d_dialog;

                    return 1;
                }
                
                static_VkActivated { playerid } = 1;

                static_VK_ID [ playerid ] = _uID;

                static_VkType [ playerid ] = 0;

            #else
                #if defined cache_get_row_int
                    new
                        Cache:qHandle = mysql_query ( g_vk_security_mysql_handle,query_string ) ;

                    static
                        _uID;

                    _uID = cache_get_field_content_int(0,!"u_userid",g_vk_security_mysql_handle ) ;

                    if ( ! _uID)
                    {
                        cache_delete ( qHandle ) ;

                        goto d_dialog;

                        return 1;
                    }
                
                    static_VkActivated { playerid } = 1;

                    static_VK_ID [ playerid ] = _uID;

                    static_VkType [ playerid ] = 0;
                #endif
            #endif

            SendClientMessage ( playerid,-1,ACTIVATION_MSG ) ;

            return cache_delete ( qHandle ) ;
        }
    }

    #if defined SECURITY_OnDialogResponse
        return SECURITY_OnDialogResponse ( playerid, dialogid, response, listitem, inputtext ) ;
    #else
        return 0;
    #endif
}

#if defined _ALS_OnDialogResponse
	#undef OnDialogResponse
#else
	#define _ALS_OnDialogResponse
#endif
#define OnDialogResponse SECURITY_OnDialogResponse
#if defined SECURITY_OnDialogResponse
	forward SECURITY_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
#endif

public OnGameModeInit()
{
    #if defined SECURITY_OnGameModeInit
        SECURITY_OnGameModeInit( ) ;
    #endif

    mysql_tquery ( g_vk_security_mysql_handle, "CREATE TABLE IF NOT EXISTS `vk_security` (`u_name` varchar(24) NOT NULL UNIQUE,`u_userid` int(11) NOT NULL DEFAULT 0,`u_code` int(11) NOT NULL DEFAULT 0,`u_type` int(11) NOT NULL DEFAULT 0,`u_ip` varchar(16) NOT NULL) ENGINE=InnoDB DEFAULT CHARSET=cp1251;", "","" ) ;

    print("-------------------------------------") ;
	print("VK Security by Examplio loaded! ") ;
	printf("Version: %i.0 ", V_VERSION) ;
	print("-------------------------------------") ;

    return 1;
}

#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit SECURITY_OnGameModeInit

#if defined SECURITY_OnGameModeInit
    forward SECURITY_OnGameModeInit( ) ;
#endif

public OnPlayerConnect ( playerid ) 
{
    #if defined SECURITY_OnPlayerConnect
        SECURITY_OnPlayerConnect ( playerid ) ;
    #endif

    static
        query_string[64 + MAX_PLAYER_NAME + 55];  

    GetPlayerName ( playerid,  static_name [ playerid ] , MAX_PLAYER_NAME ) ;

    format(query_string, sizeof(query_string), "\
        SELECT * FROM `vk_security` WHERE `u_name` = '%s'",  
        static_name [ playerid ] 
     ) ;

    #if !defined mysql_reconnect
        new
            Cache:qHandle = mysql_query ( g_vk_security_mysql_handle, query_string ) ;

        if(cache_num_rows()) {

            cache_get_value_name_int(0,!"u_type",static_VkType [ playerid ] );

            cache_get_value_name_int(0,!"u_userid",static_VK_ID [ playerid ] ) ;

            cache_get_value_name(0, !"u_ip", static_ip [ playerid ] , MAX_IP_LENGTH ) ;
            
            if(static_VK_ID [ playerid ] != 0)
                static_VkActivated { playerid } = 1;

        } else {
            format(query_string,sizeof(query_string),"\
                INSERT INTO `vk_security` (`u_name`) VALUES ('%s')",
                static_name [ playerid ] 
             ) ;
            mysql_tquery ( g_vk_security_mysql_handle,query_string,"","" ) ;
        }
    #else
        #if defined cache_get_row_int
            new
                Cache:qHandle = mysql_query ( g_vk_security_mysql_handle,query_string ) ;

            if(cache_num_rows()) {

                static_VkType [ playerid ] = cache_get_field_content_int(0,!"u_type",g_vk_security_mysql_handle ) ;

                static_VK_ID [ playerid ] = cache_get_field_content_int(0,!"u_userid",g_vk_security_mysql_handle ) ;

                cache_get_field_content(0, !"u_ip", static_ip [ playerid ] , 1 , MAX_IP_LENGTH ) ;

                if(static_VK_ID [ playerid ] != 0)
                    static_VkActivated{playerid} = 1;
                    
            } else {
                format(query_string,sizeof(query_string),"\
                    INSERT INTO `vk_security` (`u_name`) VALUES ('%s')",
                    static_name [ playerid ] 
                 ) ;
                mysql_tquery ( g_vk_security_mysql_handle,query_string,"","" ) ;
            }
        #endif
    #endif

    return cache_delete ( qHandle ) ;            
}

#if defined _ALS_OnPlayerConnect
    #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect SECURITY_OnPlayerConnect
#if defined SECURITY_OnPlayerConnect
    forward SECURITY_OnPlayerConnect ( playerid ) ;
#endif

public OnPlayerDisconnect ( playerid, reason) {
    #if defined SECURITY_OnPlayerDisconnect
        SECURITY_OnPlayerDisconnect ( playerid,reason ) ;
    #endif

    VK_DEL_SESSION ( playerid ) ;

    static_VkType [ playerid ] = 0;
    static_VK_ID [ playerid ] = 0;

    static_VkActivated { playerid } = 0;
    KillTimer ( static_Timer [ playerid ] ) ;

    static_name [ playerid ] [ 0 ] = EOS;
    static_ip [ playerid ] [ 0 ] = EOS;

    return 1;
}

#if defined _ALS_OnPlayerDisconnect
    #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect SECURITY_OnPlayerDisconnect

#if defined SECURITY_OnPlayerDisconnect
    forward SECURITY_OnPlayerDisconnect ( playerid, reason ) ;
#endif

//Timer and HTTP receiver
forward _vk_Update ( playerid ) ;
public _vk_Update ( playerid )  {

    static
        http_string[255 + 35 /* 255 - max domain length*/];

    format(http_string, sizeof(http_string), "%s/sessions/session%d.txt",DOMAIN, static_VK_ID [ playerid ] ) ;

    HTTP ( playerid, 1, http_string, "", "_vk_getRequest" ) ;
}
forward _vk_getRequest(index, response_code, data[] ) ;
public _vk_getRequest(index, response_code, data[]) {

    if(response_code >= 200 && response_code <= 299 && !strcmp(data,"1")) {

        ShowPlayerDialog(index, -1, 0, " ", " ", " ", " " ) ; // hide all dialogs

        SendClientMessage(index, -1, SUCESSFULLY_AUTH ) ; // scm

        GetPlayerIp(index, static_ip [ index ], MAX_IP_LENGTH ) ; // get ip

        static const qString[] = "UPDATE `vk_security` SET `u_ip` = %s' WHERE `u_name` = '%s'";

        static
            query_string [ sizeof qString + MAX_PLAYER_NAME + MAX_IP_LENGTH ];

        format(query_string,sizeof(query_string),qString, static_ip [ index ] , static_name [ index ] ) ;

        mysql_tquery ( g_vk_security_mysql_handle , query_string , "" , "" ) ;

        VK_DEL_SESSION ( index ) ; // Delete a session

        DeletePVar(index, !"VK_U_PVAR_STATUS" ) ;

        static_isLogged { index } = 1;

        #if defined VK_OnLoginDetected
            VK_OnLoginDetected ( index );
        #endif

        return KillTimer ( static_Timer [ index ] ) ;
    }
    else if(response_code >= 200 && response_code <= 299 && !strcmp(data,"-1")) {

        SendClientMessage ( index, -1, BLOCKED_AUTH ) ;

        KillTimer( static_Timer [ index ] ) ;

        VK_DEL_SESSION ( index ) ;

        return Kick ( index ) ;
    }
    return 1;
}
forward _SendRequest(index, response_code, data[] ) ;
public _SendRequest(index, response_code, data[]) {

    if(response_code >= 200 && response_code <= 299 && !strcmp(data,"ok")) {

        static_Timer [ index ] = SetTimerEx("_vk_Update",700,1,"i",index ) ;
        SetPVarInt(index,!"VK_U_PVAR_STATUS",1) ;
    }
    else SendClientMessage(index,-1,IS_NOT_AVAILABLE ) ;
}

#if defined PAWNRAKNET_INC_ && PAWNRAKNET_INCLUDE_VERSION >= 140
    const ORPC_SetPlayerName = 11;

    ORPC:ORPC_SetPlayerName ( playerid, BitStream:bs)
    {
        new 
            raknet_playerId, 
            raknet_nickname[MAX_PLAYER_NAME + 1];
            
        BS_ReadValue(bs, PR_UINT16, raknet_playerId, PR_STRING8, raknet_nickname, sizeof raknet_nickname ) ;

        if( strcmp ( static_name [ raknet_playerId ] , raknet_nickname ) ) {

            #if !defined mysql_reconnect 
                mysql_escape_string(raknet_nickname , raknet_nickname , sizeof raknet_nickname , g_vk_security_mysql_handle);
            #else
                mysql_escape_string(raknet_nickname , raknet_nickname , g_vk_security_mysql_handle , sizeof raknet_nickname);
            #endif

            static const qString[] = "\
                UPDATE `vk_security` SET `u_name` = '%s' WHERE `u_name` = '%s'"
            ;

            static
                query_string [ sizeof qString + MAX_PLAYER_NAME + MAX_PLAYER_NAME + 2 ] ;

            format ( query_string , sizeof ( query_string ) , qString , raknet_nickname , static_name [ raknet_playerId ] ) ;

            mysql_tquery ( g_vk_security_mysql_handle , query_string , "" , "" ) ;

            format ( static_name [ raknet_playerId ] , sizeof ( static_name[] ) , raknet_nickname ) ;
        }

        return 1;
    }
#endif