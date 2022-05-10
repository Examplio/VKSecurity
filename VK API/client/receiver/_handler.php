<?php

$name = $_GET [ 'name' ] ; 
$user = $_GET [ 'to' ] ;
$p_id = $_GET [ 'pid' ] ;

if($p_id == -1) {
    unlink("../../sessions/session" . $user . ".txt");
    exit;
}

require_once '../../client/config/config.php';
require_once '../../client/src/VK.php';

$vk = new VK ( VK_TOKEN , CONFIRM_STR );

$json = '{ "inline":true, "buttons":[ [ { "action":{ "type":"text", "payload":"{\"command\": \"confirm\"}", "label":"Подтвердить" }, "color":"positive" }, { "action":{ "type":"text", "payload":"{\"command\": \"block\"}", "label":"Заблокировать" }, "color":"negative" } ] ] }';

$nowtime = date ( "d.m.Y H:i:s" );

$f_new_session = fopen ( "../../sessions/session" . $user . ".txt" , "w+" );

fclose ( $f_new_session );

$vk -> sendButton ( $user , "Совершена попытка входа в аккаунт: {$name}.\nВремя: {$nowtime}\n\nЕсли это не Вы, значит ваш аккаунт пытаются взломать.\n\nВы можете заблокировать вход потенциального злоумышленника тогда сессия будет сброшена\n\nВыберите действие:" , $json);