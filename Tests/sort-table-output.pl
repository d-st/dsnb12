#!/usr/bin/perl

while(<STDIN>) {
  if(/^\s+level/ || /^symbol/) {
    foreach(sort(@x)) { print $_ };
    @x=();
    print $_;
  } else {
    push (@x, $_);
  }
}
foreach(sort(@x)) { print $_ };

