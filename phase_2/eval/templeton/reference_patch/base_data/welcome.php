<?php 
	if ( $_SERVER["REQUEST_METHOD"] == "GET" ) {
		$name = $_GET["name"];
		$email = $_GET["email"];
	} else if ( $_SERVER["REQUEST_METHOD"] == "POST" ) {
		$name = $_POST["name"];
		$email = $_POST["email"];
	}
?>
<html>
	<head>
		<title> CHESS Templeton PHP Welcome Page</title>
	</head>
	<body>

	Welcome <?php echo $name; ?><br>
	Your email address is: <?php echo $email; ?>

	</body>
</html>
