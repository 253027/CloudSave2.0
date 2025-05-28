CREATE DATABASE `CloudSave`;

USE `CloudSave`;

DROP TABLE IF EXISTS `UserInformation`;

CREATE TABLE `UserInformation` (
    `id` INT(11) NOT NULL AUTO_INCREMENT COMMENT '用户ID',
    `name` VARCHAR(32) NOT NULL DEFAULT '' COMMENT '用户名',
    `password` VARCHAR(32) NOT NULL DEFAULT '' COMMENT '密码',
    `salt` VARCHAR(256) NOT NULL DEFAULT '' COMMENT '混淆码',
    `email` VARCHAR(64) NOT NULL DEFAULT '' COMMENT '邮箱',
    `sex` TINYINT(1) NOT NULL DEFAULT 0 COMMENT '性别',
    `nick` VARCHAR(32) NOT NULL DEFAULT '' COMMENT '昵称',
    `avatar` VARCHAR(128) NOT NULL DEFAULT '' COMMENT '头像',
    `telphone` VARCHAR(11) NOT NULL DEFAULT '' COMMENT '手机号',
    `createTime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    `updateTime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    PRIMARY KEY (`id`),
    KEY `index_name` (`name`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

DROP TABLE IF EXISTS `RelationShip`;

CREATE TABLE `RelationShip` (
    `id` INT(11) NOT NULL AUTO_INCREMENT COMMENT '关系ID',
    `user` INT(11) NOT NULL DEFAULT '0' COMMENT '用户ID',
    `peer` INT(11) NOT NULL DEFAULT '0' COMMENT '好友ID或群ID',
    `status` TINYINT(1) NOT NULL DEFAULT '0' COMMENT '状态',
    `createTime` INT(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
    `updateTime` INT(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
    PRIMARY KEY (`id`),
    KEY `index_user_peer_status_updateTime` (
        `user`,
        `peer`,
        `status`,
        `updateTime`
    )
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

DROP TABLE IF EXISTS `Session`;

CREATE TABLE `Session` (
    `id` INT(11) NOT NULL AUTO_INCREMENT COMMENT '关系ID',
    `user` INT(11) NOT NULL DEFAULT '0' COMMENT '用户ID',
    `peer` INT(11) NOT NULL DEFAULT '0' COMMENT '好友ID或群ID',
    `type` TINYINT(1) NOT NULL DEFAULT '0' COMMENT '类型',
    `status` TINYINT(1) NOT NULL DEFAULT '0' COMMENT '状态',
    `createTime` INT(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
    `updateTime` INT(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
    PRIMARY KEY (`id`),
    KEY `index_user_peer_status_updateTime` (
        `user`,
        `peer`,
        `status`,
        `updateTime`
    )
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4;

DROP Table IF EXISTS `MessageContent`;

CREATE TABLE `MessageContent` (
    `id` INT(11) NOT NULL AUTO_INCREMENT,
    `messageId` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '消息ID',
    `relation` INT(11) NOT NULL DEFAULT '0' COMMENT '关系ID',
    `user` INT(11) NOT NULL DEFAULT '0' COMMENT '发送方',
    `peer` INT(11) NOT NULL DEFAULT '0' COMMENT '接收方',
    `type` TINYINT(1) NOT NULL DEFAULT '0' COMMENT '消息类型',
    `content` VARCHAR(4096) DEFAULT '' COMMENT '消息内容',
    `status` TINYINT(1) NOT NULL DEFAULT '0' COMMENT '0-正常 1-被删除',
    `createTime` INT(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
    PRIMARY KEY (`id`),
    KEY `index_from_to_status` (`user`, `peer`, `status`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_bin;