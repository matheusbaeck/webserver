<?php
// cgi-bin/script.php

// Set the content type
header("Content-Type: text/html");

// Get query parameters
$param1 = isset($_GET['param1']) ? htmlspecialchars($_GET['param1']) : 'not provided';
$param2 = isset($_GET['param2']) ? htmlspecialchars($_GET['param2']) : 'not provided';

// Output the parameters
echo "<html><body>";
echo "<h1>Received Parameters:</h1>";
echo "<p>Param1: $param1</p>";
echo "\n";