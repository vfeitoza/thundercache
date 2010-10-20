drop database if exists thunder;
CREATE DATABASE thunder;
USE thunder;

CREATE TABLE IF NOT EXISTS `thunder` (
  `id` bigint NOT NULL auto_increment,
  `file` varchar(767) NOT NULL,
  `domain` varchar(255) NOT NULL,
  `size` int(10) unsigned NOT NULL default '0',
  `modified` datetime NOT NULL,
  `downloaded` datetime NOT NULL,
  `requested` int(10) unsigned NOT NULL default '0',
  `last_request` datetime NOT NULL,
  `deleted` tinyint(1) NOT NULL default '0',
  `static` tinyint(1) NOT NULL default '0',
    PRIMARY KEY  (`id`),
    UNIQUE KEY `file_domain` (`file`,`domain`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

