<?
  include "header.php";

  if (file_exists("/home/groups/l/li/licq/htdocs/zorbstats/zorblogspages.php"))
  {
    include ("/home/groups/l/li/licq/htdocs/zorbstats/zorblogspages.php");
    $pagename = substr($PHP_SELF, 1);
    $dispname = substr($PHP_SELF, 1, -4);
    countpage($pagename, $dispname);
  }
?>
      
      <div id="content" > 

				<div id="main"><img src="images/main_title_info.gif" alt="Info" /></div>
        <div id="maincontent">
          <div id="subcontent">
            
            <p>Licq is a multi-threaded ICQ clone written mostly in C++. It uses an extensive plugin system to manage many different functions. The main gui is written using the Qt widget set. Licq is distributed under GPL with some special QPL exceptions for Qt.</p>

            <p>Development is very ongoing, with new features and bugs fixed frequently, so updates are available often. The current version should be very stable, but as with all software use it at your own risk.</p>
            
            <p>See the <a href="README">README</a> or <a href="faqs.php">Frequently Asked Questions</a> for more information.</p>

            <p>Get the current stable release here.</p>

            <!-- Info Item Begin-->
            <div class="news">
              <h2>ICQ Features:</h2>
              <ul>
                <li>Contact List - fully functionning online/offline user list with pixmaps and everything.</li>
                <li>Messages - send and receive messages both online and offline through the server. Send in color too.</li>
                <li>Chat - chat with other users in regular and multiparty mode, set/see colors/fonts and save chat session.</li>
                <li>File transfer - batch transfer files to and from other icq users.</li>
                <li>URL - send and receive URL's from other users.</li>
                <li>Status - set status to any of the available ones, set and retrieve away message.</li>
                <li>User info - retrieve and update all user and personal information.</li>
                <li>Whitepages - full support for General/More/Work/About info as well as searches based on this info.</li>
                <li>History - keeps a log of all messages to and from a user.</li>
                <li>Add/remove and edit user information from within the program.</li>
                <li>Invisible list - be invisible to some users all the time.</li>
                <li>Visible list - be visible to some users when you are in invisible mode.</li>
                <li>Online user notification - have a sound played when a specific user goes online</li>
                <li>Last seen online - shows the last time you've seen this user online</li>
                <li>Local time - displays the local time for the user</li>
                <li>Away-to-User - be a different status to particular users</li>
                <li>Accept-in-Away - automatically accept events from certain users in certain statuses</li>
                <li>Send contact lists - send a list of users to other users</li>
                <li>Search - full support for searching by email/uin/alias/whitepages</li>
                <li>Random chat - find random users (and be found)</li>
                <li>Server stored contacts - keep your groups and users stored on the server</li>
                <li>Typing notification - see when the remot use is typing a message to you</li>
                <li>Phone book - keep track of users phone numbers, and yours</li>
                <li>User pictures - see the picture of the user, if they have one available</li>
                <li>Proxy support - HTTPS proxy support to connect through a proxy server</li>
              </ul>
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Info Item End-->

            <!-- Info Item Begin-->
            <div class="news">
              <h2>New Features:</h2>
              <ul>
                <li>User groups - organize your users into groups.</li>
                <li>Network monitor window - allows you to see the actual data being sent and received from mirabilis and other users.</li>
                <li>Themed Docking - dock Licq using one of many different icon themes</li>
                <li>Skin and icon pack support to make the qt-gui look like anything including transparent contact list</li>
                <li>Customizable - allows full customization of colors, fonts, sizes and pixmaps for the contact list.</li>
                <li>FIFO support for remotely controlling your licq session</li>
                <li>Extensive utility support to add many new features.</li>
                <li>Extensible OnEvent system for playing sounds or running programs when messages arrive, passing user specific information as arguments.</li>
                <li>Plugin system allows for multiple possible GUIs, protocols, and other things.</li>
                <li>International support with Unicode and translations</li>
                <li>Fully multi-threaded for faster response and fewer slowdowns.</li>
                <li>User floaty windows</li>
                <li>Send urgent or to contact list with a single click</li>
                <li>Always use the correct ip even if hostname is not set correctly</li>
                <li>Option to flash all or just urgent incoming events</li>
                <li>Accept modes for when to use OnEvents</li>
                <li>Auto-raise and auto-popup incoming messages</li>
                <li>Per-user customized auto responses</li>
                <li>Statistics of how many messages you have sent & received and more</li>
                <li>Tabbed chatting</li>
                <li>SSL secure client to client connections</li>
                <li>GPG encrypted messages</li>
              </ul>
 
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Info Item End-->

            <!-- Info Item Begin-->
            <div class="news">

<H2>Requirements</H2>
<UL>
<LI><B>POSIX thread (pthreads) final draft support</B>.  Most operating systems support
    pthreads natively, however FreeBSD in particular does not completely support
    the final draft.  To build using FreeBSD, see the README.FREEBSD file included in the
    main distribution.
<LI><B>Thread-safe X11 libraries</B>.  Again, most operating systems have no problem with
    this, but if you are using libc5 under Linux (ie you get "Fatal IO Error" when starting
    Licq) you will have to make sure that
    you are using thread-safe libraries (see <A HREF="http://www.xmms.org/faq.html#r2" TARGET="_top">www.xmms.org</A>).
<LI><B>Qt 3.0</B> (libraries and developement headers).  Check out <A HREF="http://www.trolltech.com" TARGET="_top">
    http://www.trolltech.com</A> for more information.  This is only required if you
    want to use the Qt gui plugin (recommended).
<LI><B>C++</B>.  A c++ compiler and libstdc++ plus c++ headers are also required.
<LI><B>GNU Make</B>.  Comes with BSD and Linux, others will have to install it.
</UL>
<BR>
<P>RPM users will need to ensure the following packages are installed:<BR>
<UL>
<LI>qt (3.0 or greater)
<LI>libstdc++
<LI>gcc-c++
<LI>openssl (0.9.5a or greater)
</UL>
<P>Additionally if you are compiling from source you will need:<BR>
<UL>
<LI>qt-devel
<LI>libstdc++-devel
<LI>XFree86-devel
<LI>Openssl for secure connection support (optional)
<LI>GpgMe for GPG support (optional)
</UL>


              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Info Item End-->

            <!-- Info Item Begin-->
            <div class="news">

<A NAME="lml"></A>
<H2>The Licq Mailing List</H2><P>

There are five mailing lists for Licq announcements and discussion:<BR>
For more information, including how to subscribe/unsubscribe, and an archive of past
posts, follow the relevant link below.
<P>
<A HREF="http://lists.sourceforge.net/mailman/listinfo/licq-announce" TARGET="_top">licq-announce</A> - For announcements of new releases only.<BR>
<A HREF="http://lists.sourceforge.net/mailman/listinfo/licq-main" TARGET="_top">licq-main</A> - For general discussion and help.<BR>
<A HREF="http://lists.sourceforge.net/mailman/listinfo/licq-devel" TARGET="_top">licq-devel</A> - For developers/patches/cvs discussion.<BR>
<A HREF="http://lists.sourceforge.net/mailman/listinfo/licq-cvs" TARGET="_top">licq-cvs</A> - For CVS commits, a "read-only" list.<BR>
<A HREF="http://lists.sourceforge.net/mailman/listinfo/licq-bugs" TARGET="_top">licq-bugs</A> - For SF.net bug reports, very high traffic! For Developers.<BR>

              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Info Item End-->

          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
