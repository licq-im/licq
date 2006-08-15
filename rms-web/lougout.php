<?php
include "check_session.php";
session_destroy();
header("location:index.php");
?>
