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
    PRIMARY KEY (`id`), KEY `index_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `RelationShip`;
CREATE TABLE `RelationShip` (
    `id` int NOT NULL AUTO_INCREMENT COMMENT '关系ID',
    `user` int NOT NULL DEFAULT '0' COMMENT '用户ID',
    `peer` int NOT NULL DEFAULT '0' COMMENT '好友ID或群ID',
    `status` tinyint(1) NOT NULL DEFAULT '0' COMMENT '状态',
    `createTime` int unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
    `updateTime` int unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',  
    PRIMARY KEY (`id`), 
    KEY `index_user_peer_status_updateTime` (`user`,`peer`,`status`,`updateTime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `Session`;
CREATE TABLE `Session` (
    `id` int NOT NULL AUTO_INCREMENT COMMENT '关系ID',
    `user` int NOT NULL DEFAULT '0' COMMENT '用户ID',
    `peer` int NOT NULL DEFAULT '0' COMMENT '好友ID或群ID',
    `type` tinyint(1) NOT NULL DEFAULT '0' COMMENT '类型',
    `status` tinyint(1) NOT NULL DEFAULT '0' COMMENT '状态',
    `createTime` int unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',
    `updateTime` int unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',
    PRIMARY KEY (`id`),
    KEY `index_user_peer_status_updateTime` (`user`,`peer`,`status`,`updateTime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;