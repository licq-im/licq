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

my %month =
("Jan",1,"Feb",2,"Mar",3,"Apr",4,"May",5,"Jun",6,
 "Jul",7,"Aug",8,"Sep",9,"Oct",10,"Nov",11,"Dec",12 );

# Check user's version to avoid problems - implamented by security@solarweb.com
#my $version = `licq -h|head -1|awk '{print \$3}'`;
#chop($version) if $version =~ /\n$/;
#
#if ($version >= 0.71)
#{
#  print "ERROR - incorrect version\n";;
#  print "[This script was not meant for your Licq version ($version)]\n";
#  exit 1;
#}

$USER_LIST = "$ENV{'HOME'}/.licq/users.conf";
open (LIST, $USER_LIST) ||
  die "Fatal error: Unable to open '$USER_LIST', exiting.\n";

print "Updating history files.\n";

# Fetch the owner's name
$USER_INFO = "$ENV{'HOME'}/.licq/owner.uin";
$owner_name = `grep Alias $USER_INFO`;
chop($owner_name);
$owner_name =~ s/Alias =//;
$owner_name =~ s/^ *//;
$owner_name =~ s/ *$//;

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
  process_newheader();
  process_newbody();
}

# Process the owner's history file
$USER_HISTORY = "$ENV{'HOME'}/.licq/history/owner.history";
$USER_OUTPUT = "$ENV{'HOME'}/.licq/history/owner.history.new";
$USER_INFO = "$ENV{'HOME'}/.licq/owner.uin";
process_newheader();
process_newbody();


sub process_newheader
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

      #if ($dir eq "?")
      #{
      #  printf "Unknown dir: From: |$from_name| To: |$to_name| Owner: |$owner_name|\n";
      #}

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
    elsif (/.* from .* received ... ... .. ..:.. 199.:/ ||
           /.* from .* received ... ... .. ..:.. 200.:/)
    {
      print_msg();    
      $dir = "R";
      
      $subject = $_;
      $subject =~ s/ from.*//;

      $date = $_;
      $date =~ s/.* from .* received //;
      chop($date);
    }
    elsif (/.* from .* sent ... ... .. ..:.. 199.:/ ||
           /.* from .* sent ... ... .. ..:.. 200.:/)
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
  #$timestamp = `date --date '$date' +\%s`;
   my ($dw,$m,$d,$h,$min,$y) =
      ($date =~ /(\w{3}) (\w{3})\s+(\d+) (\d+):(\d+) (\d+)/ );
  $timestamp = timelocal( 0, $min, $h, $d, $month{$m}-1, $y-1900 );

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



sub process_newbody
{
  if (!open (HISTORY, $USER_HISTORY))
    { print "unable to open '$USER_HISTORY', skipping.\n"; next; }
  open (OUTPUT, ">$USER_OUTPUT") ||
    die "Fatal error: Unable to open '$USER_OUTPUT' for writing.\n";

  while(<HISTORY>)
  {
    if (/^\[/) { last; }
  }

  while(/^\[/)
  {
    chop;
    s/\?/R/;
    $cmd = $_;
    $cmd =~ s/\[ . \| //;
    $cmd =~ s/ \| .... \| .... \| [0123456789]* \]//;
    $subcmd = $_;
    $subcmd =~ s/\[ . \| .... \| //;
    $subcmd =~ s/ \| .... \| [0123456789]* \]//;

    # Message
    if ($cmd =~ /0001/)
    {
      printf OUTPUT "\n$_\n";
      while(<HISTORY>)
      {
        if (/^\[/) { last; }
        if (/^:/) { printf OUTPUT "$_"; }
      }
      next;
    }
    # Chat
    elsif ($cmd =~ /0002/)
    {
      printf OUTPUT "\n$_\n";
      # Cancelled
      if ($subcmd =~ /2000/)
      {
        printf OUTPUT "\n\n";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
        }
        next;
      }
      $_ = <HISTORY>;
      #if (/cancelled/g)
      #{
      #  printf OUTPUT "\n";
      #  while (<HISTORY>)
      #  {
      #    if (/^\[/) { next; }
      #  }
      #  last;
      #}
      # Request
      if (/^:Reason:/)
      {
        s/Reason: //;
        printf OUTPUT "$_";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
          if (/^:/) { printf OUTPUT "$_"; }
        }
        next;
      }
      elsif (/^:Chat .equest/)
      {
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
          if (/^:/) { printf OUTPUT "$_"; }
        }
        next;
      }
      else
      {
        printf OUTPUT "$_";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
          if (/^:/) { printf OUTPUT "$_"; }
        }
        next;
      }
    }
    # File transter
    elsif ($cmd =~ /0003/)
    {
      printf OUTPUT "\n$_\n";
      # Cancelled
      if ($subcmd =~ /2000/)
      {
        printf OUTPUT "\n";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
        }
        next; 
      }
      $_ = <HISTORY>;
      if (/cancelled/g)
      {
        printf OUTPUT ":\n:0\n:\n\n";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
        }
        next;
      }
      # Request
      if (/^:File:/)
      {
        my $name = $_;
        $name =~ s/^:File: //;
        $name =~ s/ \(.*\).*//;
        my $size = $_;
        $size =~ s/.*\(//;
        $size =~ s/\).*//;
        $size =~ s/ bytes//;
        printf OUTPUT ":$name:$size";
        $_ = <HISTORY>;
        s/Description: //;
        printf OUTPUT "$_";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
          if (/^:/) { printf OUTPUT "$_"; }
        }
        next;
      }
      elsif (/^:Name: /)
      {
        my $name = $_;
        $name =~ s/Name: //;
        printf OUTPUT "$name";
        $_ = <HISTORY>;
        my $size = $_;
        $size =~ s/Size: //;
        printf OUTPUT "$size";
        $_ = <HISTORY>;
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
          if (/^:/) { printf OUTPUT "$_"; }
        }
        next;
      }
      else
      {
        printf OUTPUT ":\n:0\n:\n\n";
        while (<HISTORY>)
        {
          if (/^\[/) { last; }
        }
        next;
      }
    }
    # URL
    elsif ($cmd =~ /0004/)
    {
      printf OUTPUT "\n$_\n";
      $_ = <HISTORY>;
      if (/^:URL:/)
      {
        s/URL: //;
        printf OUTPUT "$_";
        $_ = <HISTORY>;
        if (/^:Description: /)
        {
          s/Description: //;
          printf OUTPUT "$_";
        }
      }
      else
      {
        printf OUTPUT "$_:\n";
      }
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
        if (/^:/) { printf OUTPUT "$_"; }
      }
      next;
    }
    # Auth Request
    elsif ($cmd =~ /0006/)
    {
      printf OUTPUT "\n$_\n";
      $_ = <HISTORY>;
      if (/^:\(/)
      {
        s/.*from //;
        s/:.*//;
        print OUTPUT ":$_";
        $_ = <HISTORY>;
        print OUTPUT "$_";
        $_ = <HISTORY>;
        print OUTPUT "$_";
        $_ = <HISTORY>;
        print OUTPUT "$_";
        $_ = <HISTORY>;
        print OUTPUT "$_";
        <HISTORY>;
      }
      elsif (/requests authorization/)
      {
        my $uin = $_;
        $uin =~ s/, requests.*//;
        $uin =~ s/.*uin //;
        my $alias = $_;
        $alias =~ s/ \(.*//;
        my $fname = $_;
        $fname =~ s/.*\(//;
        $fname =~ s/ .*\),.*//;
        my $lname = $_;
        $lname =~ s/, .*//;
        $lname =~ s/.*\(.* //;
        my $email = $_;
        $email =~ s/\), .*//;
        $email =~ s/.*, //;
        printf OUTPUT ":$uin\n:$alias\n:$fname\n:$lname\n:$email\n";
      }
      else
      {
        printf OUTPUT ":0\n:ALIAS\n:FNAME\n:LNAME\n:EMAIL\n";
      }
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
        if (/^:/) { printf OUTPUT "$_"; }
      }
      next;
    }
    # Added to list
    elsif ($cmd =~ /0012/)
    {
      printf OUTPUT "\n$_\n";
      $_ = <HISTORY>;
      if (/^:\(/)
      {
        my $uin = $_;
        $uin =~ s/.* user //;
        $uin =~ s/ \(.*//;
        my $alias = $_;
        $alias =~ s/.*user [0123456789]* \(//;
        $alias =~ s/, .*//;
        my $fname = $_;
        $fname =~ s/.*user [0123456789]* \(//;
        $fname =~ s/\).*//;
        $fname =~ s/.*?, //;
        $fname =~ s/ .*//;
        my $lname = $_;
        $lname =~ s/.*user [0123456789]* \(//;
        $lname =~ s/\).*//;
        $lname =~ s/.*?, //;
        $lname =~ s/, .*//;
        $lname =~ s/.* //;
        my $email = $_;
        $email =~ s/.*user [0123456789]* \(.*, //;
        $email =~ s/\).*//;
        printf OUTPUT ":$uin:$alias:$fname:$lname:$email\n";
      }
      elsif (/uin [0123456789]*, added you to their/)
      {
        my $uin = $_;
        $uin =~ s/, added.*//;
        $uin =~ s/.*uin //;
        my $alias = $_;
        $alias =~ s/ \(.*//;
        my $fname = $_;
        $fname =~ s/.*\(//;
        $fname =~ s/ .*\),.*//;
        my $lname = $_;
        $lname =~ s/, .*//;
        $lname =~ s/.*\(.* //;
        my $email = $_;
        $email =~ s/\), .*//;
        $email =~ s/.*, //;
        printf OUTPUT ":$uin$alias:$fname:$lname:$email\n";
      }
      else
      {
        printf OUTPUT ":0\n:ALIAS\n:FNAME\n:LNAME\n:EMAIL\n\n";
      }
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
      }
    }
    # Web panel
    elsif ($cmd =~ /0013/)
    {
      printf OUTPUT "\n$_\n";
      $_ = <HISTORY>;
      my $name = $_;
      $name =~ s/Message from //;
      $name =~ s/ \(.*//;
      printf OUTPUT "$name";
      my $email = $_;
      $email =~ s/.* \(//;
      $email =~ s/\) .*//;
      printf OUTPUT ":$email";
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
        if (/^:/) { printf OUTPUT "$_"; }
      }
      next;
    }
    # Email pager
    elsif ($cmd =~ /0014/)
    {
      printf OUTPUT "\n$_\n";
      $_ = <HISTORY>;
      my $name = $_;
      $name =~ s/Message from //;
      $name =~ s/ \(.*//;
      printf OUTPUT "$name";
      my $email = $_;
      $email =~ s/.* \(//;
      $email =~ s/\) .*//;
      printf OUTPUT ":$email";
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
        if (/^:/) { printf OUTPUT "$_"; }
      }
      next;
    }
    # Contact list
    elsif ($cmd =~ /0019/)
    {
      printf OUTPUT "\n$_\n";
      <HISTORY>;
      <HISTORY>;
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
        my $uin = $_;
        $uin =~ s/^: *//;
        $uin =~ s/ .*//;
        my $alias = $_;
        $alias =~ s/^: *[0123456789]* //;
        printf OUTPUT ":$uin\n:$alias";
      }
      next;
    }
   
    # Something else
    else
    {
      #printf "Unknown sub-command: $cmd\n";
      while (<HISTORY>)
      {
        if (/^\[/) { last; }
        #printf OUTPUT "$_";
      }
      next;
    }
    
  }
  close(HISTORY);
  close(OUTPUT);
  `mv -f $USER_OUTPUT $USER_HISTORY`;
}
