#!/usr/bin/perl
#
# Script to convert older Licq configuration to the new one
#
#

use strict;
use IO::Handle;

STDOUT->autoflush(1);
my $date;
my $subject;
my $content;
my $dir;
my $owner_name;
my $from_name;
my $to_name;
my $timestamp;
my $subcmd;
my $cmd;

my $uin;
my $USER_HISTORY;
my $USER_INFO;
my $USER_LIST;
my $USER_OUTPUT;

$USER_LIST = "$ENV{'HOME'}/.licq/users.conf";
open (LIST, $USER_LIST) ||
  die "Fatal error: Unable to open '$USER_LIST', exiting.\n";

print "Updating history files.\n";


# Read in the number of users, which we ignore
<LIST>;
<LIST>;
while (<LIST>)
{
  chop;
  if (!/User.*= /) { next; }
  $uin = $_;
  $uin =~ s/User.*= //;
  $USER_HISTORY = "$ENV{'HOME'}/.licq/history/$uin.history";
  $USER_OUTPUT = "$ENV{'HOME'}/.licq/history/$uin.history.new";
  $USER_INFO = "$ENV{'HOME'}/.licq/users/$uin.uin";
  process_file();
}

# Process the owner's history file
$USER_HISTORY = "$ENV{'HOME'}/.licq/history/owner.history";
$USER_OUTPUT = "$ENV{'HOME'}/.licq/history/owner.history.new";
$USER_INFO = "$ENV{'HOME'}/.licq/owner.uin";
process_file();


sub process_file
{
  if (!open (HISTORY, $USER_HISTORY))
    { print "unable to open '$USER_HISTORY', skipping.\n"; next; }
  open (OUTPUT, ">$USER_OUTPUT") ||
    die "Fatal error: Unable to open '$USER_OUTPUT' for writing.\n";

  $content = "";
  $dir = "?";
  while(<HISTORY>)
  {
    chop;
    if (/.*\-\>.*\(.{13}:.*\): /)
    {
      print_msg();    
      $from_name = $_;
      $from_name =~ s/ \-\>.*//;
     
      if ($from_name eq $owner_name)
      {
        $dir = "S";
      }
      
      $to_name = $_;
      $to_name =~ s/^.*\-\> //;
      $to_name =~ s/ \({1}?.*//;

      if ($to_name eq $owner_name)
      {
        $dir = "R";
      }

      $date = $_;
      $date =~ s/^.*\(//;
      $date =~ s/\).*//;
      if (length($date) == 16)
      {
        $date = "$date 1999";
      }
     
      $subject = $_;
      $subject =~ s/.*: //;
    }
    elsif (/.* from .* received ... ... .. ..:.. 199.:/)
    {
      print_msg();    
      $dir = "R";
      
      $subject = $_;
      $subject =~ s/ from.*//;

      $date = $_;
      $date =~ s/.* from .* received //;
      chop($date);
    }
    elsif (/.* from .* sent ... ... .. ..:.. 199.:/)
    {
      print_msg();    

      $dir = "S";

      $subject = $_;
      $subject =~ s/ from.*//;

      $date = $_;
      $date =~ s/.* from .* sent //;
      chop($date);
    }
    else
    {
      $content = $content.":".$_."\n";
    }

  }
  close(HISTORY);
  close(OUTPUT);
  `mv -f $USER_OUTPUT $USER_HISTORY`;
}

sub print_msg
{
  if ($content eq "") { return; }
  subject_to_cmd();
  date_to_timestamp();

  printf OUTPUT "[ %s | %s | %s | 0000 | %ld ]\n%s\n",
         $dir, $subcmd, $cmd, $timestamp, $content;
  $content ="";
  $dir = "?";
}

sub date_to_timestamp
{
  $timestamp = `date --date '$date' +\%s`;
}

sub subject_to_cmd
{
  if ($subject =~ /Message/i)
  {
    $subcmd = "0001";
    $cmd = "2030";
  }
  elsif ($subject =~ /URL/i)
  {
    $subcmd = "0004";
    $cmd = "2030";
  }
  elsif ($subject =~ /Chat Request Cancel/i)
  {
    $subcmd = "0002";
    $cmd = "2000";
  }
  elsif ($subject =~ /Chat Request/i)
  {
    $subcmd = "0002";
    $cmd = "2030";
  }
  elsif ($subject =~ /File Transfer Cancel/i)
  {
    $subcmd = "0003";
    $cmd = "2000";
  }
  elsif ($subject =~ /File Transfer/i)
  {
    $subcmd = "0003";
    $cmd = "2030";
  }
  elsif ($subject =~ /Added to contact list/i)
  {
    $subcmd = "0012";
    $cmd = "0260";
  }
  elsif ($subject =~ /Authorization request/i)
  {
    $subcmd = "0006";
    $cmd = "0260";
  }
  elsif ($subject =~ /Web Panel/i)
  {
    $subcmd = "0013";
    $cmd = "0260";
  }
  elsif ($subject =~ /Email Pager/i)
  {
    $subcmd = "0014";
    $cmd = "0260";
  }
  elsif ($subject =~ /Contact List/i)
  {
    $subcmd = "0019";
    $cmd = "0260";
  }
  else
  {
    $subcmd = "0000";
    $cmd = "0000";
  }

}


