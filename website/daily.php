<? include('header.php'); ?>
  <div id="content" > 

      <div id="main"><img src="images/main_title_dailies.gif" alt="SVN Dailies" /></div>
        <div id="maincontent">
          <div id="subcontent">

          <p style="color:white;">.</p> <!-- Cheat when there is no text at the top of the page -->

            <!-- Screenshot Item Begin-->
            <div class="news">
             <h2>SVN snapshots</h2>
             <p>SVN snapshots, in addition to anonymous SVN (see below), are available daily. If you want to participate in development and get full SVN access, please ask <a href="mailto:jon@licq.org">Jon</a>.</p>
             <p>If you want to know what has changed in every detail, you should subscribe to the <a href="http://trac.licq.org/wiki/MailingList">Licq commit mailinglist</a>.</p>
             <p>Brief summary of changes for the last week:<br>
             <?php require("xlog.html"); ?>
             </p>

             <p>
             You can download the current Licq snapshot <a href="http://licq.sf.net/licq-daily.tar.bz2">licq-20060705</a>.<br>
             <p>Remember, these are development versions, so they might have new and untested features or contain bugs. They might not even compile, so use them at your own risk.
             <p>Please join the Licq <a href="http://trac.licq.org/wiki/MailingList">development mailinglist</a> when you use the snapshots regularly.
             You should report problems with the snapshots or
             discuss feature requests/patches there.
             <p>If there is a problem with the snapshot itself, please tell <a href="mailto:jon@licq.org">me</a>.
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Screenshot Item End-->

            <!-- Screenshot Item Begin-->
            <div class="news">
            <h2>Anonymous SVN</h2>
            <p>The anonymous SVN server is read-only. If you have installed the svn binary,
            you can use it to easily keep up to date with bleeding edge Licq development. This is 
            <b>only</b> intended for those who have at least basic knowledge in <a href="http://svnbook.org">SVN usage</a>
            or those who want to figure it out on their own. All others are strongly recommended to use
            the regularly generated SVN snapshots (see above for details).</p>

            <p>Checkout the latest version of Licq and the most common plugins with:</p>
            <pre>   # svn co http://svn.licq.org/svn/trunk/licq</pre>

            <p>You have to run</p>
            <pre>   # make -f Makefile.cvs</pre>
            <p>to generate the ./configure script needed for compiling the source. You need automake 1.4 or later installed to succeed.</p>
            <p>If you experience any unusual problems, please inform me.</p>
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Screenshot Item End-->

<br>
          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

      </div> 

<? include "footer.php" ?>
