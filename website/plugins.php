<?
  include "header.php";
?>
      
      <div id="content" > 

				<div id="main"><img src="images/main_title_plugins.gif" alt="Plugins" /></div>
        <div id="maincontent">
          <div id="subcontent">
            
            <p>Plugins form a fundamental layer in the Licq architecture. They communicate with the Licq daemon to allow user interfaces and other functions to be built. Creating a plugin is fairly tricky as there is little documentation and the daemon interface is somewhat complicated. A good place to start is the <a href="PLUGINS.HOWTO">PLUGINS.HOWTO</a>, and the Licq header files.</p>

            <p>Installing a plugin is, however, simple. First, unpack the plugin to the "plugins" directory in the main Licq source tree. Then build and install it (ie place it in /usr/local/lib/licq). Then run licq using the "-p" option and the name of the plugin (minus the leading "licq_" and tailing ".so"). For example, the qt-gui plugin may be loaded by doing<br />licq -p qt-gui<br />where the plugin has been installed as /usr/local/lib/licq/licq_qt-gui.so.</p>
            
            <p>After the first time, any plugins will load automatically at startup without the need for the -p option.</p>

            <p>A number of plugins are available for Licq at the current time:</p>

            <!-- Download Item Begin-->
            <div class="news">
              <h2>Interface Plugins</h2>

              <div class="tables">
                <table style="margin-left: 5%; width: 90%;"  border="0" cellpadding="5" cellspacing="0">
                  <tr>
                    <td style="width: 20%;"><b>Plugin</b></td>
                    <td style="width: 10%;"><b>Version</b></td>
                    <td><b>Description</b></td>
                  </tr>
                  <tr>
                    <td>Qt GUI</td>
                    <td>1.3.2</td>
                    <td>The offical interface for Licq written in Qt. Comes with the main distribution. Includes all the features of Licq as listed in the Info page.  See the plugins/qt-gui/doc directory for more information.<br />Includes full KDE support as well if configured with "--with-kde".</td>
                  </tr>
                  <tr>
                    <td>Console</td>
                    <td>1.3.2</td>
                    <td>This is a console interface plugin for Licq using ncurses.  It features multiple command windows, and much of the basic icq functionality.  It can be retrieved using CVS as described in the <a href="http://www.rhrk.uni-kl.de/~dmuell/licq">Licq cvs page</a>, the module is called "console"..</td>
                  </tr>
                  <tr>
                    <td><a href="http://icqnd.sourceforge.net/">IcQnD Gtk-2 GUI</a></td>
                    <td>0.1.6-1.31</td>
                    <td>A <B>New</B> GTK2 Gui! This one looks like it has many features already and should be used instead of the other GTK plugins.</td>
                  </tr>

                  <tr>
                    <td><a href="http://jons-gtk-gui.sourceforge.net">Jons GTK+ Gui</a></td>
                    <td>0.20</td>
                    <td>A pretty functional GTK+ interface. <b>MAINTAINER NEEDED</b></td>
                  </tr>
                  <tr>
                    <td><a href="http://gtk.licq.org/">GTK+Licq</a></td>
                    <td>0.50.1</td>
                    <td>A easy to use gui using Gtk with optional GNOME support including a nice applet.</td>
                  </tr>
                  <tr>
                    <td><a href="http://nicq.sourceforge.net">Nicq</a></td>
                    <td>0.40</td>
                    <td>An excellent Qt based GUI which uses an IRC style single window interface.</td>
                  </tr>
                </table>
              </div>

              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Download Item End-->

            <!-- Download Item Begin-->
            <div class="news">
              <h2>Additional Functionality Plugins</h2>

              <div class="tables">
                <table style="margin-left: 5%; width: 90%;"  border="0" cellpadding="5" cellspacing="0">
                  <tr>
                    <td style="width: 20%;"><b>Plugin</b></td>
                    <td style="width: 10%;"><b>Version</b></td>
                    <td><b>Description</b></td>
                  </tr>
                  <tr>
                    <td>Forwarder</td>
                    <td>1.3.2</td>
                    <td>A simple plugin to forward incoming ICQ events to either an email address or another ICQ account.  This is great for forwarding a secondary ICQ account or forwarding to a pager/mobile phone.  Also included in the main distribution.</td>
                  </tr>
                  <tr>
                    <td>Auto-Responder</td>
                    <td>1.3.2</td>
                    <td>Plugin to automatically respond to any incoming event with the output of a given program, which can be passed user arguments, or event the message text itself.  Often used in conjunction with "fortune" to send people fortunes.  Also included in the main distribution.</td>
                  </tr>
                  <tr>
                    <td>Update Hosts</td>
                    <td>0.0.5</td>
                    <td>A simple plugin to add your contact list to your /etc/hosts file, thus allowing the use of a users alias as a hostname for contacting them using ftp/telnet/talk... The plugin can be run concurrently with a gui plugin and will dynamically maintain your /etc/hosts file as users go online and offline.<br />Can be found on the ftp site (ftp.xx.licq.org) in the plugins directory.</td>
                  </tr>
                  <tr>
                    <td><a href="http://www.esquadro.com.br/~ismore/mailchecker-0.5.tar.gz">Mailchecker</a></td>
                    <td>1.0</td>
                    <td>This is a POP3 plugin and was created to notify the user when there is mail on your server. There are some bugs but the author is working to resolve them.  Please notify the contributer of problems.<br />Contributed by Ismael Orenstein &lt;perdig@linuxbr.com.br&gt;</td>
                  </tr>
                  <tr>
                    <td>Remote Management Service</td>
                    <td>1.3.2</td>
                    <td>This plugin opens a TCP server on the Licq box which allows you to telnet in and perform commands and get information.  It works with the new licqweb web interface. It acts much like the FIFO only it is two way.  Currently it supports viewing/changing status, viewing the user list, viewing groups, sending messages/urls, and dumping the network log.<br />Basic security is implemented through the need to enter your UIN and ICQ password before being allowed access to commands.<br />Written by Graham Roff.  New enhancements by Jon Keating.</td>
                  </tr>
                  <tr>
                    <td><a href="http://licq-osd.sf.net">On Screen Display</a></td>
                    <td>1.3.0</td>
                    <td>The On Screen Display (OSD) plugin uses libxosd2 to display messages on the screen as soon as they are received.  Currently under development by <a href="mailto:martinmaurer@gmx.at">Martin Maurer</a>.</td>
                  </tr>
                </table>
              </div>

              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- Download Item End-->



          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
