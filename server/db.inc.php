<?php
    $mysqli = new mysqli("localhost", "root", "", "curiositycheats");
    if(mysqli_connect_errno($mysqli))
        die("[-] connection failed");
?>