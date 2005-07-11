      
      <div id="content" > 

      <div id="main"><img src="images/main_title_faqs.gif" alt="Faqs" /></div>
        <div id="maincontent">
          <div id="subcontent">

            <!-- News Item Begin-->
            <div class="news">
              <h2>Licq FAQ</h2>
              <h3>September 30, 2004</h3>
              
              <p>
              Originally written by Graham Roff<br>
              Rewritten by Jon Keating
              </p>
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- News Item End-->
            
            <!-- News Item Begin-->
            <div class="news">
              <h2>Section 1: Basics</h2>
                <p>
                <blockquote>
                <A HREF="#1_1">1.1: Who wrote Licq?</A><BR>
                <A HREF="#1_2">1.2: What does Licq stand for anyway?</A><BR>
                <A HREF="#1_3">1.3: Is Licq based on micq code like many of the other clones?</A><BR>
                </blockquote>
                </p>
                
            <h3>Section 2: Compiling / Installing</h3>
                <blockquote>
                <A HREF="#2_1">2.1: What is needed to compile Licq?</A><BR>
                <A HREF="#2_2">2.2: How do I get Licq working through a SOCKS5 server?</A><BR>
                
                <A HREF="#2_3">2.3: How do I do a manual install (ie I have a binary distribution)?</A><BR>
                <A HREF="#2_4">2.4: I am trying to compile with Solaris and the compilation freezes at the "moc" line.</A><BR>
                <A HREF="#2_5">2.5: I am compiling under Solaris and I get an internal error.</A><BR>
                <A HREF="#2_6">2.6: I am using an older compiler (ie gcc 2.7) and get errors about ANSI C++ not allowing implicit casting of "void *".</A><BR>
                <A HREF="#2_7">2.7: What is socklen_t?</A><BR>
                </blockquote>
                
            <h3>Section 3: Using</h3>
                <blockquote>
                <A HREF="#3_1">3.1: Where do I get a uin from?</A><BR>
                <A HREF="#3_2">3.2: Does multi-party chat work?</A><BR>
                <A HREF="#3_3">3.3: What is, and how do I use, the licq fifo?</A><BR>
                <A HREF="#3_4">3.4: What happened to the "beep" OnEvent option, or, how do I make the pc-speaker beep when receiving a message?</A><BR>
                <A HREF="#3_5">3.5: Licq appears 5 times in my process list, what's going on?</A><BR>
                <A HREF="#3_6">3.6: I get "[ERR] Unable to allocate TCP port for local server (Unknown host)" when I start Licq.</A><BR>
                <A HREF="#3_7">3.7: What happened to the spoof uin option?</A><BR>
                <A HREF="#3_8">3.8: I am using ip_masq_icq module and it doesn't forward connections to Licq.</A><BR>
                <A HREF="#3_9">3.9: How do I configure ipmasq for ICQ?</A><BR>
                
                <A HREF="#3_10">3.10: I can't connect to ICQ, all I get are timeouts!</A><BR>
                <A HREF="#3_11">3.11: Does Licq support server side contact lists?</A><BR>
                </blockquote>

            <h3>Section 4: Plugins</h3>
                <BLOCKQUOTE>
                <A HREF="#4_1">4.1: I get something about being unable to load a plugin because "file not found" or "no such file or directory".</A><BR>
                <A HREF="#4_2">4.2: I get something about "undefined symbol: ..." when I try and load the Qt-GUI plugin.</A><BR>
                <A HREF="#4_3">4.3: Licq starts up but then exits with "licq: Fatal IO error: client killed".</A><BR>
                
                <A HREF="#4_4">4.4: What does "undefined symbol: LP_Main" mean?</A><BR>
                <A HREF="#4_5">4.5: Qt-gui crashes on startup, it says something about "XRegisterIMInstantiateCallback"</A><BR>
                <A HREF="#4_6">4.6: When Licq is docked it is still visible in the taskbar, what can i do?"</A><BR>
                </BLOCKQUOTE>
                
            <h3>Section 5: Qt-GUI Plugin</h3>
                <BLOCKQUOTE>
                <A HREF="#5_1">5.1: The GUI says it is unable to load &lt;file&gt;.xpm or &lt;file&gt;.gif, but the file is there!</A><BR>
                
                <A HREF="#5_2">5.2: How do I change the colors of the Qt-Gui?</A><BR>
                <A HREF="#5_3">5.3: Why are there no spaces between words in the History tab?</A><BR>
                <A HREF="#5_4">5.4: How do I dock the Licq icon?</A><BR>
                <A HREF="#5_5">5.5: How do I see other encodings (Russian, Japanese, etc.)?</A><BR>
                </BLOCKQUOTE>
                
            <h3>Section 6: Protocol Plugins</h3>
                <BLOCKQUOTE>
                <A HREF="#6_1">6.1: What protocols are available?</A><BR>
                <A HREF="#6_2">6.2: How do I start using a protocol plugin?</A><BR>
                </BLOCKQUOTE>

            <h3>Section 7: Donations</h3>
                <BLOCKQUOTE>
                <A HREF="#7_1">7.1: What are the operating expenses?</A><BR>
                <A HREF="#7_2">7.2: What is the emergency fund?</A><BR>
                <A HREF="#7_3">7.3: How do I know my donation is going to the project?</A><BR>
                
                <A HREF="#7_4">7.4: I donated money, will you add the feature I want?</A><BR>
                </BLOCKQUOTE>
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- News Item End-->
                
            <!-- News Item Begin-->
            <div class="news">
              
            <H3><STRONG>Section 1: Basics</STRONG></H3>
                <P>
                <A NAME="1_1"><STRONG>1.1: Who wrote Licq?</STRONG>
                <P>
                Licq was first begun as a text mode client to ICQ in February 1998 by
                Graham Roff and a programmer who wishes for some strange
                reason to remain anonymous.  He's from Nova Scotia, perhaps that explains
                things.  The Qt port was written by Graham Roff and starting in June
                1998.<BR><BR>
                Since then the entire code was rewritten by Graham Roff, with the
                assistance of numerous internet volunteers.  The protocol plugin
                rewrite of Licq was done by the current maintainer, Jon Keating.  All
                current development is done by Jon Keating, Thomas Reitelbach, and
                many volunteers.
                <P>
                <BR>
                
                <A NAME="1_2"><STRONG>1.2: What does Licq stand for anyway?</STRONG>
                <P>
                Contrary to popular belief, it does not stand for Linux ICQ.  This
                would be (a) a bit conceited, and (b) way too obvious a name for a linux
                program.
                <P>
                <BR>
                
                <A NAME="1_3"><STRONG>1.3: Is Licq based on micq code like many of the other clones? </STRONG>
                <P>
                Licq was begun a long time before micq was released and is based
                completely on original source code.  It is also done in C++ as opposed to C
                like micq and most of the other clones.
                <P>
                <BR>

                <H3><STRONG>Section 2: Compiling and Installing</STRONG></H3>
                <P>

                <A NAME="2_1"><STRONG>2.1: What is needed to compile Licq?</STRONG>
                <P>
                See the web page in the info section under Requirements for all the details.
                <P>
                <BR>
                
                <A NAME="2_2"><STRONG>2.2: How do I get Licq working with a SOCKS5 server?</STRONG>
                <P>
                I do not use socks, and didn't write the code to make Licq work with
                socks.  After configuring with --enable-socks5 you have to set some environment
                variables as well, follow the guidelines that come with the socks5 client.
                Other than that, I can't help you.  Try the README.SOCKS included with the
                Licq distribution.
                <P>
                <BR>
                
                <A NAME="2_3"><STRONG>2.3: How do I do a manual install (ie I have a binary distribution)?</STRONG>
                <P>
                <EM>&amp;lt;= 0.61</EM><BR>
                Easy, first copy the binary to somewhere nice (like /usr/local/bin).
                Then copy licq-base.tar.gz to /usr/local/share.  There are some other
                interesting documents to look at if you want as well.  Then run licq and it 
                should finish the install for you.  See, it's easy.<BR>
                <EM>&amp;gt;= 0.70</EM><BR>
                Copy the binary somewhere nice like above.  Then copy everything from the 
                share/ directory to /usr/local/share/licq.  To manually install a plugin simply
                copy it to /usr/local/lib/licq/plugins.
                
                <P>
                <BR>
                
                <A NAME="2_4"><STRONG>2.4: I am trying to compile and the compilation freezes at the "moc" line.</STRONG>
                <P>
                Licq requires GNU make to compiler properly.  Alternatively,
                you can compile all the moc files by hand doing:<BR>
                $&gt; moc &lt;infile&gt; -o &lt;outfile&gt;<BR>
                The list of files to moc is in the Makefile.  If you do this, you will
                still have to replace the $^ in the following section:<BR>
                
                ++ licq: $(OBJECTS) $(METAOBJ)<BR>
                ++         $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)<BR>
                to be:<BR>
                ++ licq: $(OBJECTS) $(METAOBJ)<BR>
                ++         $(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(METAOBJ) $(LDFLAGS) $(LDLIBS)<BR>
                <P>
                <BR>
                
                <A NAME="2_5"><STRONG>2.5: I am compiling and I get an internal error.</STRONG>
                <P>
                This is a problem with some versions of g++.  Simply remove <EM>-O2</EM> 
                
                from the relevant Makefile and it should work fine.
                <P>
                <BR>
                
                <A NAME="2_6"><STRONG>2.6: I get errors about ANSI C++ not allowing implicit casting of "void *".</STRONG>
                <P>
                Some older implementations of libstdc++ erroneously define NULL as being
                ((void *)0), which causes some c++ compilers to complain.  If this is the case,
                you can either upgrade to a newer compiler or simply run the "remove-null.sh" (included
                with the licq-data package in <EM>contrib</EM>)
                inside the <EM>src/</EM> directory, which will replace all NULL's with zeros.
                <P>
                <BR>
                
                <A NAME="2_7"><STRONG>2.7: What is socklen_t?</STRONG>
                
                <P>
                If you get compile errors refering to socklen_t simply edit config.h and
                add "#define socklen_t int" to it somewhere.
                <P>
                <BR>
                
                
                <H3><STRONG>Section 3: Using</STRONG></H3>
                <P>

                
                <A NAME="3_1"><STRONG>3.1: How do I get a uin?</STRONG>
                <P>
                Licq can register a new user with Mirabilis.  Just select "Register User" from
                the user menu.  See http://www.mirabilis.com for more info.
                <P>
                <BR>
                
                <A NAME="3_2"><STRONG>3.2: Does multi-party chat work?</STRONG>
                <P>
                Yes.
                <P>
                <BR>
                
                <A NAME="3_3"><STRONG>3.3: What is, and how do I use, the licq fifo?</STRONG>
                <P>
                
                Please see the <A HREF="README.FIFO">README.FIFO</A>
                <P>
                <BR>
                
                <A NAME="3_4"><STRONG>3.4: What happened to the "beep" OnEvent option, or, how do I make the pc-speaker beep when receiving a message?</STRONG>
                <P>
                The beep command did not work properly and so was incorporated into a
                self-contained executable (build beep.c in the contrib directory using the
                instructions contained therein) which can be called instead of "play" (or
                whatever) in order to generate a beep.<BR>
                So to rephrase, instead of a beep option, you can build a "beep" executable,
                and enter "beep" as the OnEvent Command (the parameters are irrelevant for
                this command).
                <P>
                <BR>
                
                <A NAME="3_5"><STRONG>3.5: Licq appears 5 times in my process list, what's going on?</STRONG>
                
                <P>
                There is really only 1 copy of Licq running, using the resources of one of
                the listed processes.  The reason you see multiple copies is that Linux
                shows threads as separate processes.  As Licq may be running 5 threads or more
                at a time, multiple process entries will appear.
                <P>
                <BR>
                
                <A NAME="3_6"><STRONG>3.6: I get "[ERR] Unable to allocate TCP port for local server (Unknown host)" when I start Licq.</STRONG>
                <P>
                This will happen if you have not set up your hostname properly.  This is a
                necessary step for running many network applications.  Make sure your hostname
                (which can be found by typing "hostname") resolves to a valid ip.  Typically
                this involves adding it to /etc/hosts and setting the ip to your static ip or
                to 127.0.0.1 if you don't have one.
                <P>
                <BR> 
                
                <A NAME="3_7"><STRONG>3.7: What happened to the spoof uin option?</STRONG>
                <P>
                This is no longer supported by the ICQ protocol. Sorry.
                <P>
                <BR>
                
                
                <A NAME="3_8"><STRONG>3.8: I am using ip_masq_icq module and it doesn't forward connections to Licq.</STRONG>
                <P>
                The ip_masq_icq module uses the ip contained in the logon packet to determine whether
                or not to forward connections to your machine.  This IP is determined typically
                by looking at the hostname of the local machine.  If your hostname is not set correctly
                or cannot be resolved, then Licq will default to using 127.0.0.1, which will be ignored
                by the module.<BR>
                The solution is to ensure that your hostname (to find it, type "hostname" at the prompt)
                resolves to your correct sub-net IP.  To find out, ping your hostname ("ping &lt;hostname&gt;")
                and see what the ip ends up being.<BR>
                This ip will probably be something in the 192.168.xx.xx or 10.xx.xx.xx range.  If it
                resolves to 127.0.0.1 or 0, then there is a problem.  To fix this, simply edit
                <I>/etc/hosts</I> and enter the correct ip and hostname there, for example:
                <P>
                127.0.0.1    localhost<BR>
                
                192.168.0.2  &lt;hostname&gt;<BR>
                ...<BR>
                <P>
                <BR>
                
                <A NAME="3_9"><STRONG>3.9: How do I configure ipmasq for ICQ?</STRONG>
                <P>
                Configuring ipmasq and ipchains for ICQ is fairly simple.  The relevant
                man pages contain a good source of information as well.  Here is a sample
                configuration using ipmasq:<BR>
                ---(cut here)---<BR>
                <BR>
                # Enable port forwarding for ICQ Client<BR>
                
                #<BR>
                port=2000<BR>
                while [ $port -lt 2020 ]<BR>
                do<BR>
                /usr/sbin/ipmasqadm portfw -a -P tcp -L $xnet $port -R 192.168.1.2 $port<BR>
                port=$((port+1))<BR>
                done<BR>
                /usr/sbin/ipmasqadm portfw -a -P udp -L $xnet 4000 -R 192.168.1.2 4000<BR>
                
                <BR>
                ---(cut here)---<BR>
                <BR>
                And here is an example using iptables which is used with the Linux Kernel 2.4.x:<BR>
                ---(cut here)---<BR>
                <BR>
                iptables -t nat -A PREROUTING -p tcp --destination-port 2000:2020 -i ppp0 -j DNAT --to 192.168.1.2<BR>
                iptables -t nat -A PREROUTING -p udp --destination-port 4000 -i ppp0 -j DNAT --to 192.168.1.2<BR>
                
                <BR>
                ---(cut here)---<BR>
                
                <BR>
                This script will forward tcp ports 2000-2020 and udp port 4000 to your
                machine inside the firewall. Note $xnet is the ip address issued to you by
                your ISP and 192.168.1.2 should be changed to the ip address of the
                machine that you are running Licq on.  Also note that this is just one
                sample configuration and there are other ways to do this, using ipchains
                for example.<BR>
                <P>
                <BR>
                
                
                <A NAME="3_10"><STRONG>3.10: I can't connect to ICQ, all I get are timeouts!</STRONG>
                <P>
                This is a problem with the ICQ servers and people who use icq2000.  For
                some reason a selection of the icq servers do not allow you to log on using
                another client after using icq2000.<BR>
                The solution?  Edit ~/.licq/licq.conf and set the servers to be a valid
                icq.mirabilis.com ip except 205.188.153.*, eg 205.188.179.33.  Try
                "nslookup icq.mirabilis.com" for more.<BR>
                This, hopefully, will make it work.  In any case, it's not a Licq problem, but
                a mirabilis stupidity.<BR>
                <P>
             
                
                <BR>
                
                <A NAME="3_11"><STRONG>3.11: Does Licq support server side contact lists?</STRONG>
                <P>
                
                Yes, it is enabled by default. Licq supports having a user in multiple groups
                but the server only allows one group per user. This is a little difficult to
                manage, so there is a seperate menu item to change the server group of a user.
                Just right click a user in the Qt-GUI and take a look for yourself.
                <P>
                <BR>
                
                
                <H3><STRONG>Section 4: Plugins</STRONG></H3>
                <P>

                <A NAME="4_1"><STRONG>4.1: I get something about being unable to load a plugin because "file not found" or "no such file or directory".</STRONG>
                <P>
                If this happens with QT-GUI then you most likely configured --with-kde support. 
                In this case, please type "licq -p kde-gui" to start licq and read the README.KDE file.<BR>
                In all other cases the plugin may be unable to find a necessary library.
                Use "ldd" on the offending plugin to see which library it can't find.  Then add
                the directory containing that missing library to your /etc/ld.so.conf file or
                LD_LIBRARY_PATH environment variable (see 4.2).
                <P>
                <BR>
                
                <A NAME="4_2"><STRONG>4.2: I get something about "undefined symbol: ..." when I try and load the Qt-GUI plugin.</STRONG>
                <P>
                This problem occurs if you have a copy of qt 1.4 somewhere on your system
                and Licq is linked to it.  Verify this using <EM>ldd /usr/local/lib/licq/licq_qt-gui.so</EM>
                and see which qt library it's linked to.  If it is qt 1.4 then this library will have
                to be moved to somewhere after the location of the qt 2.0 library, as listed
                in /etc/ld.so.conf.  Read the README-QT-1.4-AND-2.0 for more information.<BR>
                One other solution is simply to add the directory with Qt 2.0 in it to your
                LD_LIBRARY_PATH environment variable like so:<BR>
                <EM>export LD_LIBRARY_PATH=/usr/local/qt/lib</EM><BR>
                which can be added to /etc/profile as per the Qt installation instructions.
                <P>
                <BR>
                
                <A NAME="4_3"><STRONG>4.3: Licq starts up but then exits with "licq: Fatal IO error: client killed".</STRONG>
                
                <P>
                Because Licq is multi-threaded, it requires thread-safe X11 libraries.  These
                can be aquired by recompiling your X server with the thread option or by grabbing
                the binaries (see the Info section of the Licq web page under Requirements).
                <P>
                <BR>
                
                <A NAME="4_4"><STRONG>4.4: What does "undefined symbol: LP_Main" mean?</STRONG>
                <P>
                This is a bug with the dynamic loader used in SMP machines.  Try changing DLOPEN_POLICY
                in config.h to RTLD_NOW and rebuild.  Or upgrade your kernel/ld.
                <P>
                <BR>
                <A NAME="4_5"><STRONG>4.5: Qt-gui crashes on startup, it says something about "XRegisterIMInstantiateCallback"</STRONG>
                <P>
                There is a bug in QT 3.x whith X Input Methods (XIM).<BR>
                Try to start licq like this: "licq -- -noxim".
                
                <P>
                <BR>
                <A NAME="4_6"><STRONG>4.6: When Licq is docked it is still visible in the taskbar, what can i do?</STRONG>
                <P>
                When you are using KDE you can prevent Licq from beeing visible in the taskbar.<BR>
                Simply prepend "kstart --skiptaskbar" to your commandline:<BR>
                <TT>kstart --skiptaskbar licq</TT>
                <P>
                <BR>
                
                
                <H3><STRONG>Section 5: Qt-GUI Plugin</STRONG></H3>
                <P>

                <A NAME="5_1"><STRONG>5.1: The QUI says it is unable to load &lt;file&gt;.xpm or &lt;file&gt;.gif, but the file is there!</STRONG>
                <P>
                Qt has problems loading certain formats of xpm.  The solution is simply
                to save the pixmap in question as a gif and edit the skin file appropriately.	 If the
                file in question is a gif, then make sure that you compiled qt with gif support
                enabled (<EM>configure -gif</EM>).
                <P>
                <BR>
                
                <A NAME="5_2"><STRONG>5.2: How do I change the colors of the Qt-Gui?</STRONG>
                <P>
                
                Colors are now integrated into the skin file.  To change colors you must
                edit the skin file (found in <EM>/usr/local/share/licq/qt-gui/skin.&lt;name&gt;</EM>) and
                add the following:<BR>
                <BR>
                # --- colors ---<BR>
                colors.online = #ff0000 <BR>
                colors.away = dark green<BR>
                colors.offline = firebrick<BR>
                colors.newuser = yellow<BR>
                colors.background = default<BR> 
                
                colors.gridlines = black<BR>
                <BR>
                Of course you get to select your own colors!
                <P>
                <BR>
                
                <A NAME="5_3"><STRONG>5.3: Why are there no spaces between words in the History tab?</STRONG>
                <P>
                This seems to be a bug in the Qt widget being used.  The solution is simply to use a TrueType font.
                Select one from the Options Dialog in the Font section.
                <P>
                <BR>
                
                <A NAME="5_4"><STRONG>5.4: How do I dock the Licq icon?</STRONG>
                <P>
                
                Docking the licq qt-gui status icon is fairly simple to do.  Here is a list
                of window managers and what to do. If your window manager supports the
                freedesktop.org standard, just enable the "Small Icon" in the Options. If you
                are using KDE, use the KDE-GUI (./configure --with-kde for the Qt-GUI).
                
                If you aren't using a freedesktop.org compliant window manager, do the following:<BR>
                <BR>
                1. Start Licq and enable the dock icon in the Options screen.<BR>
                2. Select "Save Settings" from the system menu.<BR>
                3. Restart Licq (very important).<BR>
                <BR>
                <STRONG>WindowMaker</STRONG><BR>
                4. Drag the status icon to the dock and voila.<BR>
                Note that the other icon that appears is from windowmaker and can be set
                to something nice, or disable entirely using the windowmaker menu.<BR>
                <BR>
                
                <STRONG>AfterStep</STRONG><BR>
                4. Add the following to your ~/GNUstep/Library/AfterStep/wharf file:<BR>
                "*Wharf licq      - Swallow "LicqWharf" echo &amp;"<BR>
                5. Next time start Licq, the icon should appear in the wharf.<BR>
                4*.Another possibility is to add:<BR>
                *FvwmButtons(Swallow (UseOld,NoKill,NoClose) "LicqWharf" Nop, Action (Mouse 1) 'Exec
                /usr/bin/licq')<BR>
                to your FvwmButtons config file.<BR>
                <BR>
                
                <STRONG>KDE</STRONG><BR>
                4. Configure the qt-gui plugin with --with-kde and then licq will dock in Kicker.<BR>
                5. Start Licq and there it goes!<BR>
                <BR>
                <STRONG>Gnome</STRONG><BR>
                4. Pick a smaller dock icon then the default (either 64x48 or one of the themed icons).<BR>
                5. Select "Swallow App" from the gnome panel menu.<BR>
                6. Set "Title =  LicqWharf", "Width =  64", "Height = 48".<BR>
                <BR>
                <P>
                
                <A NAME="5_5"><STRONG>5.5: How do I see other encodings (Russian, Japanese, etc.)?</STRONG><BR>
                <P>
                In the options, you can select a default encoding.  Also, there are per-user encodings. In the event window, there is an encoding button in the top right corner.  If the proper encoding isn't listed there, select the "Show all encodings" in the options dialog. You may have to experiment to find the correct encoding (there are 3 Japanese ones), but it does work properly. So please try all the appropriate encodings before saying that it doesn't work.
                </P>
                <BR>
                
                <H3><STRONG>Section 6: Protocol Plugins</STRONG></H3>
                <P>
                
                <A NAME="6_1"><STRONG>6.1: What protocols are available?</STRONG><BR>
                <P>
                Currently ICQ, AIM and MSN are available. Only one account per protocol can be used with the 1.3.0 release.  This limitation will be removed in a future release.  ICQ and AIM are viewed as one protocol from Licq, so this means only one AIM or ICQ account can be used.  Fortunately, this isn't too bad of a problem since ICQ and AIM work together.  This means you can add AIM users to your ICQ list and vice versa.  As for MSN, it is still being developed and supports basic functionality, but will be a full featured client soon!<BR><BR>
                Other protocols such as Yahoo will come later.. unless someone else starts to work on it, then sooner.
                </P>
                <BR>
                
                <A NAME="6_2"><STRONG>6.2: How do I start using a protocol plugin?</STRONG><BR>
                <P>
                First you need to install the protocol plugin.  If you installed Licq from source, it can be found in the 'plugins' directory.  After it is installed, you need to load the plugin.  From the Qt-GUI this is done from the Plugin Manager dialog.  After it is loaded, open up the Owner Manager and add your account for the new protocol.
                </P>
                <BR>
                
                <H3><STRONG>Section 7: Donations</STRONG></H3>
                <P>

                <A NAME="7_1"><STRONG>7.1: What are the operating expenses?</STRONG><BR>
                <P>
                The mail server and DNS server are currently run by Dennis Tenn and he pays all costs privately.  A percentage of the donations will go to pay for the cost of running this server.  Also, the domain names must be paid for every few years.
                </P>
                <BR>
                
                <A NAME="7_2"><STRONG>7.2: What is the emergency fund?</A></STRONG><BR>
                <P>
                It is a percentage of donations that are kept in case a main developer has a personal emergency that requires monetary assistance.  The usage for this money will be for necessary items such as food, rent, medicine, and other similiar items.  It will not be used for unnecessary items such as alcohol, hardware, or anything that is a personal interest.
                
                </P>
                <BR>
                
                <A NAME="7_3"><STRONG>7.3: How do I know my donation is going to the project?</A></STRONG><BR>
                <P>
                Since you are using Licq, you trust us not to put a backdoor in the program.  With this reasoning, you should trust us that your valued contribution will be used solely to directly help Licq.
                </P>
                <BR>
                
                <A NAME="7_4"><STRONG>7.4: I donated money, will you add the feature I want?</A></STRONG><BR>
                <P>
                Just because you donated money does not entitle you to automatically have a feature you want to be added.  It will be considered as all other feature requests are considered.  Your donations are appreciated, but they are viewed as contribuations you make to help continue the Licq project.
                </P>
                <BR>
              <div class="footer">
                <div class="footer_right"></div>
              </div>
            </div>
            <!-- News Item End-->

          </div>
        </div>

        <div id="mainfooter"><p id="mainfooter_left"></p><!-- <img src="images/main_bg_bottomleft.gif" border="0" alt="Main" /> --></div>

			</div> 

<? include "footer.php" ?>
