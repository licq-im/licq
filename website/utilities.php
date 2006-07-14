<?
  include "header.php";
?>
      
      <div id="content" > 

				<div id="main"><img src="images/main_title_utilities.gif" alt="Utilites" /></div>
        <div id="maincontent">
          <div id="subcontent">
            
            <p>Utilities are a simple way to extend the functionality of Licq. A utility is simply a short text file (see the <a href="UTILITIES.HOWTO">UTILITIES.HOWTO</a>) which defines an external program to run, and what variables to pass it. The utility can be run for any user, so for example the Pine utility will start up a terminal running pine and open a new message with the email address of the particular user in the "To:" field.</p>

            <p>Installing a utility is simple. Simply copy the .utility file into {licq prefix}/share/licq/utilities and restart licq.</p>

            <p>A number of utilities are available for Licq at the current time (new submissions are welcome):</p>

            <!-- Utility Item Begin-->
            <div class="news">
              <h2></h2>
              <div class="tables">
                <table style="margin-left: 5%; width: 90%;"  border="0" cellpadding="5" cellspacing="0">
                  <tr>
                    <td style="width: 30%;"><b>utility</b></td>
                    <td><b>description</b></td>
                  </tr>
                  <tr>
                    <td>hostname</td>
                    <td>performs a hostname lookup on the user's ip</td>
                  </tr>
                  <tr>
                    <td>pine</td>
                    <td>email the user using pine</td>
                  </tr>
                  <tr>
                    <td>ping</td>
                    <td>pings the user's ip</td>
                  </tr>
                  <tr>
                    <td>finger</td>
                    <td>finger the remote user</td>
                  </tr>
                  <tr>
                    <td>netscape</td>
                    <td>shows the user's homepage in netscape (uses "viewurl-netscape.sh")</td>
                  </tr>
                  <tr>
                    <td>talk</td>
                    <td>sends a talk request to the user</td>
                  </tr>                  <tr>
                    <td>ftp</td>
                    <td>ftp's to the user's system using unix ftp</td>
                  </tr>
                  <tr>
                    <td>queso</td>
                    <td>uses <a href="http://apostols.org/projectz/queso/" >queso</a> to try to determine the user's operating system</td>
                  </tr>
                  <tr>
                    <td>nmap</td>
                    <td>uses <a href="http://www.insecure.org/nmap" >nmap</a> to scan for open ports</td>
                  </tr>
                  <tr>
                    <td>vnc</td>
                    <td>opens a <a href="http://www.realvnc.com/">vnc</a> viewer of the remote user's window</td>
                  </tr>
                  <tr>
                    <td>traceroute</td>
                    <td>traces the ip route to the user</td>
                  </tr>
                  <tr>
                    <td>xtraceroute</td>
                    <td>a graphical traceroute program. shows the packets route in
                  a 3d map of the planet. needs opengl (or mesa). way cool. 8)
                  (http://www.dtek.chalmers.se/~d3august/xt/)</td>
                  </tr>
                  <tr>
                    <td>mutt</td>
                    <td>uses <a href="http://www.mutt.org" >mutt</a> to email the user</td>
                  </tr>
                  <tr>
                    <td>gftp</td>
                    <td>a cute, graphical ftp client.  (http://gftp.seul.org)</td>
                  </tr>
                  <tr>
                    <td>backorifice</td>
                    <td>opens a backorifice connection to the user (http://cvs.linux.hr/boclient/).</td>
                  </tr>
                </table>
              </div>

              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Utility Item End-->
      
          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
