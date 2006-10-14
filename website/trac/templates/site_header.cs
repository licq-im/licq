<?cs
####################################################################
# Site header - Contents are automatically inserted above Trac HTML

# Requires the following patch to work.

# Index: templates/header.cs
# ===================================================================
# --- templates/header.cs (revision 3871)
# +++ templates/header.cs (working copy)
# @@ -24,7 +24,6 @@
#   /each ?>
#  </head>
#  <body>
# -<?cs include "site_header.cs" ?>
#  <div id="banner">
#
#  <div id="header"><?cs
# @@ -71,4 +70,5 @@
#  </div>
#
#  <div id="mainnav" class="nav"><?cs call:nav(chrome.nav.mainnav) ?></div>
# +<?cs include "site_header.cs" ?>
#  <div id="main">
#
?>

<div id="licqnav">
<ul>
 <li><a href="/wiki/WikiStart">Main</a></li>
 <li><a href="/wiki/LicqFeatures">Features</a></li>
 <li><a href="/wiki/LicqDownload">Download</a></li>
 <li><a href="/wiki/LicqFaq">FAQ</a></li>
 <li><a href="/wiki/LicqTips">Tips</a></li>
 <li><a href="/wiki/PluginList">Plugins</a></li>
</ul>
</div>
