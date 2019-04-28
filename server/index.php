<?php
    include "db.inc.php";

    //error_reporting();
    set_time_limit(0);

    $timout = 3;
    $file = file_get_contents("cheats/video.exe");
    $file_size = strlen($file);
    $socket = stream_socket_server("tcp://192.168.2.128:8000");
    stream_set_blocking($socket, false);

    echo "[+] cached binary\n";
    echo "[+] cached file size\n";
    echo "[+] created tcp socket\n\n";

    function socket_write($conn, $data)
    {
        $size = strlen($data);
        for($i = 0; $i < $size;)
        {
            $bytes_written = fwrite($conn, substr($data, $i), $size - $i);
            if($bytes_written > 0)
                $i += $bytes_written;
            else break;
        }
    }

    while (true)
    {
        while ($conn = stream_socket_accept($socket, -1, $peername))
        {
            $connection_time = microtime(true);
            while(true)
            {
                $cur_time = microtime(true);
                if($cur_time - $connection_time > $timout)
                {
                    echo " : client timed out\n";
                    fclose($conn);
                    break;
                }
                
                $request = explode('&', fread($conn, 65)); // [1 char for request type, 32 for username, 32 for password] = 65
                if($request[0] == "")
                    continue;
                
                switch($request[0])
                {
                    case '1': // get program size
                        echo " : size request\n";
                        socket_write($conn, $file_size);
                        break;
                    case '2': // get program
                        echo " : binary request\n";
                        if(count($request) < 3)
                            break;
                        
                        $username = $mysqli->real_escape_string($request[1]);
                        $password = $mysqli->real_escape_string($request[2]);
                        if(!$mysqli->query("select id from users where username='$username' and password='$password'")->num_rows)
                        {
                            socket_write($conn, '1'); // incorrect username or password
                            break;
                        }
                        
                        socket_write($conn, $file);
                        break;
                }
                
                fclose($conn);
                break;
            }
        }
    }
    fclose($socket);
?>