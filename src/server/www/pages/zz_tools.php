<?

function generate_router_key () {
	$tmp = "";
	for ($i = 0; $i < 64; ++$i) {
		$tmp .= sprintf ("%x", mt_rand(0,15));
	}

	return $tmp;
}

?>
