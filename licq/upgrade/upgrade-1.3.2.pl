#!/usr/bin/perl
#
# Script to convert older Licq configuration to the new one
#
#

use strict;
use IO::Handle;

STDOUT->autoflush(1);
my $uin;
my $USER_LIST;
my $USER_INFO;
my $USER_OUTPUT;
my $LICQ_PATH="$ENV{'HOME'}";

print "Licq 1.3.2 Upgrade Script\n\n";
print "This script will upgrade licq's config files to be compatible\n";
print "with version 1.3.2. We will use the licq config files located\n";
print "in $LICQ_PATH.  It will permamently change these files.\n";
print "!!! IT IS WISE TO MAKE A BACKUP BEFORE CONTINUING !!!\n";
print "Continue (y/N)? ";
my $r = <STDIN>;

if (!($r =~ /^[yY].*/))
{
  print "Aborted.\n";
  exit;
}

$USER_LIST = "$LICQ_PATH/.licq/users.conf";
open (LIST, $USER_LIST) ||
  die "Fatal error: Unable to open '$USER_LIST', exiting.\n";

print "Updating user files...\n";

# Read in the number of users, which we ignore
<LIST>;
<LIST>;
while (<LIST>)
{
  chop;
  if (!/User.*= .*Licq$/) { next; }
  $uin = $_;
  $uin =~ s/User.*= //;
  $USER_INFO = "$LICQ_PATH/.licq/users/$uin";
  process_updatealias();
}

print "\nCompleted!\n";
print "If there were any problems, manually update the nick by updating\n";
print "their user info.\n\n";
print "Enjoy Licq v1.3.2. Keep an eye out on http://blog.licq.org\n";
print "for news about more releases, bug fixes, new plugins, etc.\n\n";

sub process_updatealias
{
  my $encodingLine = `grep -m 1 Encoding $USER_INFO`;
  my $newAlias = "";
  $encodingLine =~ s/^.*[ ]*=[ ]*(.*)/$1/;
  $encodingLine =~ s/ //;
  `grep "Alias =" $USER_INFO > $USER_INFO.nick`;
  if ($encodingLine =~ /./)
  {
    $newAlias = `iconv -f "$encodingLine" -t "UTF-8" $USER_INFO.nick`;
  }
  else
  {
    $newAlias = `iconv -t "UTF-8" $USER_INFO.nick`;
  }

  if (! open(INFO, $USER_INFO))
  { 
    print "Unable to open '$USER_INFO', skipping.\n";
    next;
  }

  $USER_OUTPUT=$USER_INFO.".temp";

  open(OUTPUT, ">$USER_OUTPUT") ||
    die "Fatal error: Unable to open '$USER_OUTPUT' for writing.\n";

  while (<INFO>)
  { 
     if (/^Alias/)
     {
       printf OUTPUT $newAlias;
     }
     else
     {
       printf OUTPUT $_;
     }
  } 

  close(INFO);
  close(OUTPUT);
  `mv -f $USER_INFO.temp $USER_INFO`;
  `rm -f $USER_INFO.nick`;
}
