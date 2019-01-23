<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()" />
    </xsl:copy>
  </xsl:template>

  <!-- BuohComicList -->
  <xsl:template match="template[@parent = 'GtkBin']">
    <object>
      <!-- GtkBin is abstract -->
      <xsl:attribute name="class">GtkWindow</xsl:attribute>
      <xsl:apply-templates select="@*[name() != 'parent' and name() != 'class']|node()" />
    </object>
  </xsl:template>
  <!-- Other templates -->
  <xsl:template match="template[@parent != 'GtkBin']">
    <object>
      <xsl:attribute name="class">
        <xsl:value-of select="@parent"/>
      </xsl:attribute>
      <xsl:apply-templates select="@*[name() != 'parent' and name() != 'class']|node()" />
    </object>
  </xsl:template>

  <!-- Usage of custom elements -->
  <xsl:template match="object/@class[. = 'BuohView']">
    <!-- GtkBin is abstract -->
    <xsl:attribute name="class">GtkWindow</xsl:attribute>
  </xsl:template>
  <xsl:template match="object/@class[. = 'BuohComicList']">
    <xsl:attribute name="class">GtkNotebook</xsl:attribute>
  </xsl:template>
  <xsl:template match="object/@class[. = 'BuohViewComic']">
    <xsl:attribute name="class">GtkViewport</xsl:attribute>
  </xsl:template>
  <xsl:template match="object/@class[. = 'BuohViewMessage']">
    <xsl:attribute name="class">GtkViewport</xsl:attribute>
  </xsl:template>
</xsl:stylesheet>
