<?php
include "check_session.php";
session_destroy();
header("Location: index.php");
?>
