<?php

require_once('_config.php');

// MYSQL

define("host","host_change_me"); // Host
define("user","user_change_me"); // User
define("password","password_change_me"); // Password
define("db","DB_changeme"); // DB

if($user_id <= 0)
    exit;

// EN: Variables | RU: Переменные

$cmd = getCmd();
$args = getMessage(3);

// EN: Commands | RU: Команды

if($payload['command'] == 'confirm') { // EN: Calls when user press 'Confirm' | RU: Вызывается когда пользователь тыкает 'Подтвердить'

    if(!file_exists("sessions/session" . $user_id . ".txt")) {
        $vk->sendMessage($user_id,"В данный момент нет открытых сессий.");
        exit;
    }

    $f_session = fopen("sessions/session" . $user_id . ".txt","r+");

    fwrite($f_session,"1");

    fclose($f_session);

    $vk->sendMessage($user_id,"Вы успешно подтвердили вход.");
    exit;
} 

if($payload['command'] == 'block') { // EN: Calls when user press 'Block' | RU: Вызывается когда пользователь тыкает 'Заблокировать'

    if(!file_exists("sessions/session" . $user_id . ".txt")) {
        $vk->sendMessage($user_id,"В данный момент нет открытых сессий.");
        exit;
    }

    $f_session = fopen("sessions/session" . $user_id . ".txt","r+");

    fwrite($f_session,"-1");

    fclose($f_session);

    $vk->sendMessage($user_id,"Вы успешно заблокировали вход.");
    exit;
}

if($args[0] == '/link' && $args[1] != '' && is_numeric($args[2]) && $args[2] >= 100000 && $args[2] <= 999999) {

    $sql_connection = mysqli_connect(HOST,USER,PASSWORD,DB);
    mysqli_select_db($sql_connection,DB);

    if (mysqli_connect_errno()) {
        $vk->sendMessage($user_id,"Connection failed: " . mysqli_connect_error());
        exit();
    }

    $squery = mysqli_query($sql_connection,"SELECT * FROM `vk_security` WHERE `u_name` = '{$args[1]}' LIMIT 1");

    if(!mysqli_num_rows($squery))
        exit;
    
    $qdata = mysqli_fetch_row($squery);

    if($args[1] == $qdata[0] && $args[2] == $qdata[2]) {
        mysqli_query($sql_connection,"UPDATE `vk_security` SET `u_userid` = '{$user_id}',`u_code` = '0' WHERE `u_name` = '{$args[1]}'");

        $vk->sendMessage($user_id,"Вы успешно привязали профиль ВКонтакте к игровому аккаунту ✅");
    }
    unset($qdata);
    unset($squery);
    exit;
}
else if($args[0] == '/unlink' && $args[1] != '' && is_numeric($args[2]) && $args[2] >= 100000 && $args[2] <= 999999)
{
    $sql_connection = mysqli_connect(HOST,USER,PASSWORD,DB);
    mysqli_select_db($sql_connection,DB);

    if (mysqli_connect_errno()) {
        $vk->sendMessage($user_id,"Connection failed: " . mysqli_connect_error());
        exit();
    }

    $squery = mysqli_query($sql_connection,"SELECT * FROM `vk_security` WHERE `u_name` = '{$args[1]}' LIMIT 1");

    if(!mysqli_num_rows($squery))
        exit;

    $qdata = mysqli_fetch_row($squery);

    if($qdata[1] != 0) {
        if($args[1] == $qdata[2]) {
            mysqli_query($sql_connection,"UPDATE `vk_security` SET `u_userid` = '0',`u_code` = '0' WHERE `u_name` = '{$args[1]}'");

            $vk->sendMessage($user_id,"Вы успешно отвязали профиль ВКонтакте от своего игрового аккаунта 🚫");
        }
    }
    exit;
}
else {
    $vk->sendMessage($user_id,"Если Вы хотите привязать/отвязать профиль, используйте:\n\n/link [Игровой ник] [Код]\n/unlink [Игровой ник] [Код]");
    exit;
}

// Functions
function getMessage($limit = null) {
    global $message;
  
    return ($limit != null) ?
           explode(" ", $message, $limit) :
           explode(" ", $message);
}
function getCmd() {
    $message = getMessage()[0];
    $first   = mb_substr($message, 0, 1);
    if (in_array($first, ['/', '!'])) {
      $cmd = mb_substr($message, 1);
      return mb_strtolower($cmd);
    }
}