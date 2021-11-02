<?php

require_once('simplevk-master/autoload.php');

const VK_KEY = "YOUR KEY"; // EN: Group token with messages permission | RU: Токен группы с правами на сообщения
const VERSION = "5.131"; // EN: API version. Leave 5.131 for correct work | RU: Версия API. Оставь 5.131 для корректной работы
const CONFIRM_STR = "YOUR CONFIRM KEY"; // EN: Confirmation String. | RU: Строка подтверждения

use DigitalStar\vk_api\vk_api;
use DigitalStar\vk_api\Execute;
$vk = vk_api::create(VK_KEY, VERSION)->setConfirm(CONFIRM_STR);
$vk = new Execute($vk);
$data = $vk->initVars($id, $message, $payload, $user_id, $type); // EN: Initializing Variables | RU: Инициализация переменных
