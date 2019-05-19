<?php

echo "ip: ".$_SERVER['SERVER_ADDR']."\n";
echo "ip: ".$_SERVER['LOCAL_ADDR']."\n";
echo "ip: ".gethostbyname(gethostname())."\n";

?>