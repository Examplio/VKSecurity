<?php

/**
 * Written by Examplio with ❤
 * VK: vk.com/vv.kasko
 */

class VK {
    protected $data = [];
    protected $token;
    protected $v;
    protected $confirm;

    public function __construct ( $token = null, $confirm = null , $v = '5.131') {

        if((function_exists('getallheaders') && isset(getallheaders()['X-Retry-Counter'])) || isset($_SERVER['HTTP_X_RETRY_COUNTER']))
            exit('ok');

        $this->token = $token;
        $this->confirm = $confirm;
        $this->v = $v;

        $this->data = json_decode(file_get_contents('php://input'), true);

        if($this->data['type'] == 'confirmation') {
            exit($this->confirm);
        } else {
            $this->ok();

            return $this;
        }

        return $this;
    }

    public function request ( $method , $params = [] ) {
        if(!is_null($params['message']) && is_null($params['random_id'])) {
            $params['random_id'] = 0;
        }

        $params['v'] = $this->v;
        $params['access_token'] = $this->token;

        $ch = curl_init();

        curl_setopt($ch, CURLOPT_URL, "https://api.vk.ru/method/" . $method);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $params);

        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Content-Type:multipart/form-data'
        ]);
        $data = json_decode(curl_exec($ch), true);
        curl_close($ch);

       return (isset($data['error'])) ? $data : $data['response'];
    }

    public function reply ( $user_id , $message ) {
        return $this->request('messages.send',['user_id' => $user_id,'message' => $message,'random_id' => 0]);
    }

    public function sendButton ( $user_id , $message , $json ) {
        return $this->request('messages.send',['user_id' => $user_id,'message' => $message,'random_id' => 0,'keyboard' => $json]);
    }

    public function vars ( &$peer_id = null , &$user_id = null , &$message = null , &$payload = null ) {
        $peer_id = $this->data['object']['message']['peer_id'];
        $user_id = $this->data['object']['message']['from_id'];

        $message = $this->data['object']['message']['text'];
        $payload = json_decode($this->data['object']['message']['payload'],true);

        return $this->data;
    }

    protected function ok ( ) {
        if (ob_get_contents())
            ob_end_clean();

        if (is_callable('fastcgi_finish_request')) {
            echo 'ok';
            session_write_close();
            fastcgi_finish_request();
            return 1;
        }
        ignore_user_abort(true);

        ob_start();
        header('Content-Encoding: none');
        header('Content-Length: 2');
        header('Connection: close');
        echo 'ok';
        ob_end_flush();
        flush();
    }
}