print "Enter a temperature in Celsius:\n";
$celsius = <STDIN>;
chomp($celsius);

if ($celsius =~ m/^[-+]?[0-9]+(\.[0-9]*)?$/) {
	$f = ($celsius * 9 / 5) + 32;
	print "$celsius C is $f F\n";
} else {
	print "Expecting a number, so I don't understand \"$celsius\".\n";
}
