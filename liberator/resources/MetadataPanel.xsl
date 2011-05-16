<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:template match="Category">
<panel version="1" type="custom_panel">
  <xsl:attribute name="title">$$$/FitsLiberator/Metadata/Category/<xsl:value-of select="@Name"/>=Astro <xsl:value-of select="@Name"/></xsl:attribute>
  group( spacing: gSpace, horizontal: align_fill, vertical: align_top ) {
      <xsl:apply-templates select="Keyword"/>
  }
</panel>
  </xsl:template>

  <xsl:template match="Keyword">
    group( placement: place_row, horizontal: align_fill ) {
      static_text(
        font: font_big_right,
        vertical: align_center,
        name: '$$$/FitsLiberator/Metadata/Keyword/<xsl:value-of select="@Name"/>=<xsl:value-of select="@Name"/>'
      );
      <xsl:choose>
        <xsl:when test="@Type = 1">
      popup(
        horizontal: align_fill,
        items: '$$$/FitsLiberator/Completion/<xsl:value-of select="@Name"/>=<xsl:apply-templates select="Completion"/>',
        xmp_namespace: '<xsl:value-of select="//@Schema"/>',
        xmp_path: '<xsl:value-of select="@Name"/>'
      );
        </xsl:when>
        <xsl:when test="@Flags=4 or @Flags=8 or @Flags=36 or @Flags=72 or @Flags=25">
      cat_container_edit_text(
        preserve_commas: true,
        horizontal: align_fill,
        xmp_namespace: '<xsl:value-of select="//@Schema"/>',
        xmp_path: '<xsl:value-of select="@Name"/>'
      );
        </xsl:when>
        <xsl:otherwise>
      edit_text(
        horizontal: align_fill,
        xmp_namespace: '<xsl:value-of select="//@Schema"/>',
        xmp_path: '<xsl:value-of select="@Name"/>'
      );
        </xsl:otherwise>
      </xsl:choose>
    }
  </xsl:template>

  <xsl:template match="Completion">
    <xsl:value-of select="."/>{<xsl:value-of select="."/>};
  </xsl:template>
</xsl:stylesheet>