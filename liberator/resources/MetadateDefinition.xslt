<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:template match="Category">
    <div class="category">
      <h2>
        <xsl:value-of select="@Name"/>
      </h2>
      <ol>
        <xsl:apply-templates select="Keyword"/>
      </ol>
    </div>
  </xsl:template>

  <xsl:template match="Keyword">
    <li class="keyword">
      <p>
        <b class="name">
          <xsl:value-of select="@Name"/>
        </b>
        <br/>
        <xsl:value-of select="@Description"/>
      </p>
      <xsl:call-template name="CompletionList"/>
    </li>
  </xsl:template>

  <xsl:template name="CompletionList">
    <xsl:if test="count(Completion)">
    <p>
      Valid values include:
      <ul>
        <xsl:apply-templates select="Completion"/>
      </ul>
    </p>
    </xsl:if>
  </xsl:template>

  <xsl:template match="Completion">
    <li>
      <xsl:value-of select="."/>
    </li>
  </xsl:template>
  
  <xsl:template match="/">
    <html>
      <head>
        <title>Virtual Repository Metadata Tags (Version <xsl:value-of select="Metadata/@Version"/>)</title>
        <style type="text/css">
          body { font-family: sans-serif; text-align: justify; }
          h1, h2, h3 { text-align: left; }
          h1 { margin-bottom: 0; text-align: center; }
          ul { margin-top: 0; }
          .category h2 { border-bottom: 1px solid gray; }
          .keyword { margin-left: 4em; }
          .name { padding-right: 2em; }
          .version { text-align: center; font-style: italic; }
        </style>
      </head>
      <body>
        <h1>
          Virtual Repository Metadata Tags
        </h1>
        <div class="version">
          Version <xsl:value-of select="Metadata/@Version"/>
        </div>        
        <xsl:apply-templates select="Metadata/Category"/>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>
