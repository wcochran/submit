#!/usr/bin/perl

use strict;

my $nameNext = 0;
my $emailNext = 0;

for (<>) {
    next unless m{<td\s*>(.*)</td>};
    my $data = $1;
    if ($data =~ /^(\d+)$/) {
	$data = substr($data,1) if $data =~ /^0/;
	print "$data,";
	$nameNext = 1;
    } elsif ($nameNext) {
	print "\"$data\",";
	$nameNext = 0;
	$emailNext = 1;
    } elsif ($emailNext) {
	print "\"$data\"\n";
	$emailNext = 0
    }
}
