<?php

$name = $_GET['name']; 
$user = $_GET['to'];
$p_id = $_GET['pid'];

if($p_id == -1) {
    unlink("./sessions/session" . $user . ".txt");
    exit;
}

require_once('_config.php');

$YES = $vk->buttonText('Подтвердить', 'green', ['command' => 'confirm']);
$NO = $vk->buttonText('Заблокировать', 'red', ['command' => 'block']);

$nowtime = date("d/m/Y H:i:s");

$f_new_session = fopen("./sessions/session" . $user . ".txt","w+");

fclose($f_new_session);

$vk->sendButton($user, "Совершена попытка входа в аккаунт: {$name}.\nВремя: {$nowtime}\n\nЕсли это не Вы, значит ваш аккаунт пытаются взломать.\n\nВы можете заблокировать вход потенциального злоумышленника тогда сессия будет сброшена\n\nВыберите действие:", [[$YES,$NO]],true);