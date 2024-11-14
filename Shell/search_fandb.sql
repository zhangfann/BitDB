CREATE DATABASE search_fandb
CHARACTER SET utf8;
USE search_fandb;

CREATE TABLE `markdown`  (
  `id` int NOT NULL,
  `title` varchar(255) NULL,
  `content` text(25500) NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `nested_list`  (
  `parent_id` int NOT NULL,
  `child_id` int NOT NULL,
  `id` int NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

