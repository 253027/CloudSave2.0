CREATE DATABASE IF NOT EXISTS `FileServer`;
USE `FileServer`;

CREATE TABLE IF NOT EXISTS `UserInfo` 
(
    `id` INT AUTO_INCREMENT PRIMARY KEY COMMENT '用户ID',
    `username` VARCHAR(50) NOT NULL COMMENT '用户名',
    `password` VARCHAR(255) NOT NULL COMMENT '密码',
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

DROP TABLE IF EXISTS `Files`;
CREATE TABLE IF NOT EXISTS `Files`
(
    `file_id` INT AUTO_INCREMENT PRIMARY KEY COMMENT '文件ID',
    `user_id` INT NOT NULL COMMENT '上传文件的用户ID',
    `file_name` VARCHAR(255) NOT NULL COMMENT '文件名',
    `file_size` BIGINT NOT NULL COMMENT '文件大小（字节）',
    `upload_time` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '文件上传时间',
    -- `file_path` VARCHAR(1024) NOT NULL COMMENT '文件的存储路径',
    -- `is_public` BOOLEAN DEFAULT FALSE COMMENT '文件是否公开可访问',
    -- `description` TEXT COMMENT '文件描述（可选）',
    `hash_value` VARCHAR(64) COMMENT '文件的哈希值（用于文件校验）',
    FOREIGN KEY (`user_id`) REFERENCES `UserInfo`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


