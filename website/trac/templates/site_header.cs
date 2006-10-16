<?cs
####################################################################
# Site header - Contents are automatically inserted above Trac HTML

# Requires ../header.cs.patch to work.

?>

<div id="licqnav">
<ul>
 <li><a href="/wiki/WikiStart">Main</a></li>
 <li><a href="/wiki/LicqNews">News</a></li>
 <li><a href="/blog">Blog</a></li>
 <li><a href="/wiki/LicqFeatures">Features</a></li>
 <li><a href="/wiki/LicqScreenshots">Screenshots</a></li>
 <li><a href="/wiki/LicqDownload">Download</a></li>
 <li><a href="/wiki/LicqFaq">FAQ</a></li>
 <li><a href="/wiki/LicqTips">Tips</a></li>
 <li><a href="/wiki/LicqSupport">Support</a></li>
 <li><a href="/wiki/PluginList">Plugins</a></li>
 <li><a href="/wiki/LicqUtilities">Utilities</a></li>
 <li><a href="/wiki/DeveloperArea">Developers</a></li>
 <li><a href="/wiki/LicqTeam">Credits</a></li>
</ul>

<form id="licqsearch" action="<?cs var:trac.href.search ?>" method="get">
 <?cs if:trac.acl.SEARCH_VIEW ?><div>
  <input type="text" id="proj-search" name="q" size="10" accesskey="f" value="" />
  <input type="submit" value="Search" />
  <input type="hidden" name="wiki" value="on" />
  <input type="hidden" name="changeset" value="on" />
  <input type="hidden" name="ticket" value="on" />
 </div><?cs /if ?>
</form>

</div>
