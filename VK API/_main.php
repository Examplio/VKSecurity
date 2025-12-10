<?php

require_once './client/config/config.php'; // конфиг
require_once './client/src/VK.php';

$vk = new VK ( VK_TOKEN , CONFIRM_STR ); // Версия API по умолчанию 5.131

$data = $vk -> vars ( $peer_id , $user_id , $message , $payload ); // инициализация переменных

$chat_id = $peer_id - 2000000000;

if($chat_id > 0) exit;

if($payload['command'] == 'confirm') { // Нажал "да"
    
    if(!file_exists("././sessions/session" . $user_id . ".txt")) {
        $vk -> reply ( $user_id , "В данный момент нет открытых сессий." );
        exit;
    }

    $f_session = fopen ( "././sessions/session" . $user_id . ".txt" , "r+" );

    fwrite($f_session,"1");

    fclose($f_session);

    $vk->reply($user_id,"Вы успешно подтвердили вход.");
    exit;
}

if($payload['command'] == 'block') { // нажал "нет"

    if(!file_exists("./sessions/session" . $user_id . ".txt")) {
        $vk -> reply ( $user_id , "В данный момент нет открытых сессий." );
        exit;
    }

    $f_session = fopen ( "./sessions/session" . $user_id . ".txt" , "r+" );

    fwrite ( $f_session , "-1" );

    fclose ( $f_session );

    $vk -> reply ( $user_id , "Вы успешно заблокировали вход." );
    exit;
}

// Работа с текстом

$messages = explode(" ", $message);

$hd = ($message[0] == '/' || $message[0] == '!') ? true : false;

$cmd = mb_strtolower(str_replace(array("/", "!"), "", $messages[0]));

$args = array_slice($messages, 1);

if($cmd == 'link') {
    if(is_null($args[0])) {
        $vk -> reply ( $user_id , "Введите ник в игре." );
        exit;
    }

    if(!is_numeric($args[1])) {
        $vk -> reply ( $user_id , "Введите код полученный в игре." );
        exit;
    }

    // инициализация бд
    $sql_connection = mysqli_connect(HOST,USER,PASSWORD,DB);
    mysqli_select_db($sql_connection,DB);

    $name = mysqli_real_escape_string($sql_connection,$args[0]);

    if (mysqli_connect_errno()) {
        $vk -> reply ( $user_id , "Connection failed: " . mysqli_connect_error ( ) );
        exit;
    }

    $squery = mysqli_query ( $sql_connection , "SELECT * FROM `vk_security` WHERE `u_name` = '{$name}' LIMIT 1" );

    if(!mysqli_num_rows($squery))
        exit;
    
    $qdata = mysqli_fetch_row($squery);

    if($args[0] == $qdata[0] && $args[1] == $qdata[2]) {
        mysqli_query($sql_connection,"UPDATE `vk_security` SET `u_userid` = '{$user_id}',`u_code` = '0' WHERE `u_name` = '{$name}'");

        $vk -> reply ( $user_id , "Вы успешно привязали профиль ВКонтакте к игровому аккаунту ✅" );
    }
    exit;
}

if($cmd == 'unlink') {
    if(!is_string($args[0])) {
        $vk -> reply ( $user_id , "Введите ник в игре." );
        exit;
    }

    if(!is_numeric($args[1])) {
        $vk -> reply ( $user_id , "Введите код полученный в игре." );
        exit;
    }

    $sql_connection = mysqli_connect(HOST,USER,PASSWORD,DB);
    mysqli_select_db($sql_connection,DB);

    if (mysqli_connect_errno()) {
        $vk -> reply ( $user_id , "Connection failed: " . mysqli_connect_error( ) );
        exit;
    }

    $name = mysqli_real_escape_string($sql_connection,$args[0]);

    $squery = mysqli_query ( $sql_connection , "SELECT * FROM `vk_security` WHERE `u_name` = '{$name}' LIMIT 1" );

    if(!mysqli_num_rows ( $squery ))
        exit;

    $qdata = mysqli_fetch_row($squery);

    if($qdata[1] != 0) {
        if($args[1] == $qdata[2]) {
            mysqli_query($sql_connection,"UPDATE `vk_security` SET `u_userid` = '0',`u_code` = '0' WHERE `u_name` = '{$name}'");

            $vk -> reply ( $user_id , "Вы успешно отвязали профиль ВКонтакте от своего игрового аккаунта 🚫" );
        }
    }
    exit;
}