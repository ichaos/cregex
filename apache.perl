$in = 0;
#print "test";
while ($line = <>) {
	#print $line;
	if ($line =~ m/<VirtualHost(?: \*\:[0-9]*)+>\s*/) {
		#print "in";
		$in = 1;
	}
	if ($line =~ m/<\/VirtualHost>/) {
		#print "out";
		$in = 0;
	}

	if ($in eq 1) {
		if (!($line =~ m/#/) && $line =~ m/(?:ServerName|ServerAlias|DocumentRoot) ((?:(?:[\w\/.-]+) ?)+)/) {
			print $1;
			print "\n";
		}
	}
}