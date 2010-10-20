<?php

if (!( $db = new PDO('mysql:host=localhost;dbname=thunder-serial', 'root','thundercache') ) ) {
		die("Não consegui conectar no database");
}

$query = "SELECT * FROM serial WHERE serial='".$_POST['serial']."';";
//echo "$query\n";
foreach ($db->query($query) as $valor) {
	// campos na tabela: serial e child
	// nessa ordem :P
	$serial = $valor[0];
	$child = $valor[1];
}

if(empty($serial)) {
	echo "NO";
} else {
	echo "$child";
}
?>
