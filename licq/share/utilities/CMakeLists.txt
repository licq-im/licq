set(utility_FILES
  BackOrifice.utility
  finger.utility
  ftp.utility
  gftp.utility
  hostname.utility
  kmail.utility
  mutt.utility
  nmap.utility
  pine.utility
  ping.utility
  queso.utility
  speakfreely.utility
  talk.utility
  traceroute.utility
  vnc.utility
  xtraceroute.utility
)

install(FILES ${utility_FILES} DESTINATION "${Licq_SHARE_DIR}/utilities")
