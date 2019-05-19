<?php
    $mysqli = new mysqli("mysql", "root", "curiosity2702.", "curiosity");
    if(mysqli_connect_errno($mysqli))
        die("[-] connection failed");
?>