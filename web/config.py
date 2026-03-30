"""Centralised configuration - all tunables read from environment variables."""

from pydantic import Field
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(
        env_prefix="BITCRUSHER_", env_file=".env", extra="ignore"
    )

    # CORS - comma-separated list of allowed origins.
    # Set BITCRUSHER_CORS_ORIGINS="https://yourdomain.com" in production.
    cors_origins: list[str] = Field(
        description="Allowed CORS origins",
    )

    # Concurrent search limit - defaults to all logical CPUs.
    max_concurrent_searches: int = Field(
        default=0,  # 0 means os.cpu_count() at startup
        ge=0,
        description="Max parallel /search calls (0 = cpu_count)",
    )

    # LRU cache sizes for the pure-function wrappers.
    legal_moves_cache_size: int = Field(default=4096, ge=1)
    evaluate_cache_size: int = Field(default=4096, ge=1)

    # Search defaults / limits.
    default_search_depth: int = Field(default=12, ge=1, le=50)
    max_search_depth: int = Field(default=50, ge=1, le=100)
    default_time_limit_ms: int = Field(default=10_000, ge=100)
    max_time_limit_ms: int = Field(default=60_000, ge=100)

    # Rate limiting (requests per minute per IP, 0 = disabled).
    rate_limit_per_minute: int = Field(default=60, ge=0)


settings = Settings()
