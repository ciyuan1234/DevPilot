-- DevPilot V1 建表脚本（对应 SDD 第 7 节数据库设计）
-- 用法：mysql -u devpilot -p devpilot < backend/db/schema.sql

CREATE TABLE IF NOT EXISTS `user` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `username` VARCHAR(64) NOT NULL,
    `password_hash` VARCHAR(255) NOT NULL,
    `create_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_username` (`username`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci;

-- 项目表（SDD 第 7 节）：storage_type 枚举约束，list 按 user_id 走索引
CREATE TABLE IF NOT EXISTS `project` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `user_id` BIGINT UNSIGNED NOT NULL,
    `name` VARCHAR(128) NOT NULL,
    `storage_type` ENUM ('local', 'object_storage', 'remote') NOT NULL DEFAULT 'local',
    `storage_reference` VARCHAR(512) NOT NULL DEFAULT '',
    `language` VARCHAR(32) NOT NULL DEFAULT '',
    `create_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `idx_user_id` (`user_id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci;
