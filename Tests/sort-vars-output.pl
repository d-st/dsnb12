#!/usr/bin/perl

while(<STDIN>) {
	if(/^$/) {
		push(@blocks, join('#', @x));
		@x = ();
	}
	$_ =~ s/'.+'/''/;
	push(@x, $_);
}
push(@blocks, join('#', @x));

foreach(sort(@blocks)) {
	print split(/#/, $_);
}
