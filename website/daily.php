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

      <div id="main"><img src="images/main_title_dailies.gif" alt="CVS Dailies" /></div>
        <div id="maincontent">
          <div id="subcontent">

          <p style="color:white;">.</p> <!-- Cheat when there is no text at the top of the page -->
            
            <!-- Screenshot Item Begin-->
            <div class="news">
             <h2>CVS snapshots</h2>
             <p>CVS snapshots, in addition to anonymous CVS (see below), are available daily. If you want to participate in development and get full CVS access, please ask <a href="mailto:jon@licq.org">Jon</a>.</p>
             <p>If you want to know what has changed in every detail, you should subscribe to the Licq CVS commit mailinglist.To do so, write an email to <a href=" mailto:licq-cvs-request@lists.sourceforge.net">licq-cvs-request@lists.sourceforge.net</a> and put 'subscribe' in the subject.</p>
             <p>Brief summary of changes for the last week:
             <pre><?php require("xlog.html"); ?></pre></p>
             <p>
             You can download the current Licq snapshot <a href="licq-daily.tar.bz2">licq-20050529</a>.<br>
             <p>Remember, these are development versions, so they might have new and untested features or contain bugs. They might not even compile, so use them at your own risk.
             <p>Please join the Licq development mailinglist when you use the snapshots regularly.
             You should report problems with the snapshots or
             discuss feature requests/patches there. To subscribe, write an email to <a href="mailto:licq-devel-request@lists.sourceforge.net">licq-devel-request@lists.sourceforge.net</a>.
             <p>If there is a problem with the snapshot itself, please tell <a href="mailto:jon@licq.org">me</a>.
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Screenshot Item End-->

            <!-- Screenshot Item Begin-->
            <div class="news">
            <h2>Anonymous CVS</h2>
            <p>Thanks to Billy Biggs, there is now an anonymous CVS server available which is now being kept on SourceForge.
            The anonymous CVS server is read-only. If you have installed the cvs binary,
            you can use it to easily keep up to date with bleeding edge Licq development. This is 
            <b>only</b> intended for those who have at least basic knowledge in CVS usage
            or those who want to figure it out on their own. All others are strongly recommended to use
            the regularly generated CVS snapshots. See above for details.</p>
            <p>Login to the anonymous CVS server by typing</p>
            <pre>
   cvs -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/licq login
   cvs -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/licq co &lt;modulename&gt;
            </pre>
            <p>the requested password is empty, just type return.</p>
            <p>You can checkout the following modules:
            <ul>
              <li>licq</li>
              <li>licq-data</li>
              <li>licq-plugins</li>
            </ul>
            <p>You have to run
            <pre>   make -f Makefile.cvs</pre><p>to generate the ./configure script needed for compiling the source. You need automake 1.4 or later installed to succeed.<br>
            The recommended ~/.cvsrc for CVS usage is:
            <pre>
   cvs -l -z4 -q
   diff -u1 -b -p
   update -dP
   checkout -P
            </pre></p>
            <p>If you experience any unusual problems, please inform me.
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Screenshot Item End-->

<br>
            Last Modified: <?require("modify.html")?>
<br>
          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

      </div> 

<? include "footer.php" ?>
