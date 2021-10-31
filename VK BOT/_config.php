<?php

require_once('simplevk-master/autoload.php');

const VK_KEY = "6767e16916fb13c0bd32e7f0c2ac3722970813f99406afbbc083d8ce95407a37111a250afb875a16e4ba7"; // EN: Group token with messages permission | RU: Токен группы с правами на сообщения
const VERSION = "5.131"; // EN: API version. Leave 5.131 for correct work | RU: Версия API. Оставь 5.131 для корректной работы
const CONFIRM_STR = "f75ffd9a"; // EN: Confirmation String. | RU: Строка подтверждения

use DigitalStar\vk_api\vk_api;
use DigitalStar\vk_api\Execute;
$vk = vk_api::create(VK_KEY, VERSION)->setConfirm(CONFIRM_STR);
$vk = new Execute($vk);
$data = $vk->initVars($id, $message, $payload, $user_id, $type); // EN: Initializing Variables | RU: Инициализация переменных
