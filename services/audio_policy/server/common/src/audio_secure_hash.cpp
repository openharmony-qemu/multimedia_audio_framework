/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "audio_secure_hash.h"
#include <cstring>
#include <cstdint>
#include <cerrno>   // for EINVAL
#include <cstdlib>  // for size_t
#include "audio_log.h"
#include "audio_utils.h"

// ---------------- macros (right-rotate-based, standard) ----------------
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n)  ((x) >> (n))

#define Ch(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define Sigma0(x) (ROTR((x), 30) ^ ROTR((x), 19) ^ ROTR((x), 10))
#define Sigma1(x) (ROTR((x), 26) ^ ROTR((x), 21) ^ ROTR((x), 7))
#define sigma0(x) (ROTR((x), 25) ^ ROTR((x), 14) ^ SHR((x), 3))
#define sigma1(x) (ROTR((x), 15) ^ ROTR((x), 13) ^ SHR((x), 10))

namespace OHOS {
namespace AudioStandard {
static constexpr size_t BYTES_PER_WORD = 4;
static constexpr size_t BITS_PER_BYTE = 8;
static constexpr size_t DEFAULT_NUM_WORDS = 8;
static constexpr size_t DEFAULT_SIZE_16 = 16;
static constexpr size_t DEFAULT_HASH_SIZE = 32; // 256 bits
static constexpr size_t DEFAULT_BLOCK_SIZE = 64; // 512 bits
static constexpr size_t DEFAULT_MSG_SCHEDULE = 16;
static constexpr size_t DEFAULT_ROUNDS_PER_BLOCK = 64; // 64 rounds per block
static constexpr size_t DEFAULT_PAD_POSITION = 56; // position for length

// ---------------- constants ----------------
static const uint32_t K256[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

// ---------------- context ----------------
struct AlgoCTX {
    uint32_t h[DEFAULT_NUM_WORDS];
    unsigned char data[DEFAULT_BLOCK_SIZE];
    uint32_t num;            // bytes currently in data[]
    uint64_t nbits;    // total bits processed
    uint32_t mdLen;         // not used externally, kept for compatibility
};

static void AlgoInit(AlgoCTX *c);
static bool AlgoUpdate(AlgoCTX *c, const unsigned char *data, size_t len);
static bool AlgoFinal(unsigned char *md, AlgoCTX *c);
static void AlgoTransform(AlgoCTX *ctx, const unsigned char *in, size_t numBlocks);

// ---------------- public interface ----------------
unsigned char* AudioSecureHash::AudioSecureHashAlgo(const unsigned char *d, size_t n, unsigned char *md)
{
    CHECK_AND_RETURN_RET(d != nullptr || n == 0, nullptr); // illegal state, d = nullptr && n > 0
    static unsigned char staticBuffer[DEFAULT_HASH_SIZE];
    if (md == nullptr) {
        md = staticBuffer;
    }
    
    AlgoCTX ctx;
    AlgoInit(&ctx);

    // initialize buffer area to zero (not strictly necessary but keep safe)
    CHECK_AND_RETURN_RET(memset_s(ctx.data, sizeof(ctx.data), 0, sizeof(ctx.data)) == EOK, nullptr);
    ctx.num = 0;
    ctx.nbits = 0;

    CHECK_AND_RETURN_RET(AlgoUpdate(&ctx, d, n), nullptr);

    CHECK_AND_RETURN_RET(AlgoFinal(md, &ctx), nullptr);

    return md;
}

// ---------------- trans ----------------
static void AlgoTransform(AlgoCTX *ctx, const unsigned char *in, size_t numBlocks)
{
    // This is the classic scalar block processing implementation.
    while (numBlocks--) {
        uint32_t W[DEFAULT_MSG_SCHEDULE];
        uint32_t a;
        uint32_t b;
        uint32_t c;
        uint32_t d;
        uint32_t e;
        uint32_t f;
        uint32_t g;
        uint32_t h;
        uint32_t T1;
        uint32_t T2;
        const unsigned char *data = in;

        // load 16 words (big-endian)
        for (int i = 0; i < DEFAULT_MSG_SCHEDULE; ++i) {
            W[i] = (static_cast<uint32_t>(data[BYTES_PER_WORD * i]) << 24) |
                   (static_cast<uint32_t>(data[BYTES_PER_WORD * i + 1]) << 16) |
                   (static_cast<uint32_t>(data[BYTES_PER_WORD * i + 2]) << 8) |
                   (static_cast<uint32_t>(data[BYTES_PER_WORD * i + 3]));
        }

        a = ctx->h[0];
        b = ctx->h[1];
        c = ctx->h[2];
        d = ctx->h[3];
        e = ctx->h[4];
        f = ctx->h[5];
        g = ctx->h[6];
        h = ctx->h[7];

        for (size_t i = 0; i < DEFAULT_ROUNDS_PER_BLOCK; ++i) {
            uint32_t Wt;
            if (i < DEFAULT_MSG_SCHEDULE) {
                Wt = W[i];
            } else {
                uint32_t s0 = sigma0(W[(i + 1) & 0x0f]);
                uint32_t s1 = sigma1(W[(i + 14) & 0x0f]);
                Wt = (W[i & 0x0f] += s0 + s1 + W[(i + 9) & 0x0f]);
            }

            T1 = h + Sigma1(e) + Ch(e, f, g) + K256[i] + Wt;
            T2 = Sigma0(a) + Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        ctx->h[0] += a;
        ctx->h[1] += b;
        ctx->h[2] += c;
        ctx->h[3] += d;
        ctx->h[4] += e;
        ctx->h[5] += f;
        ctx->h[6] += g;
        ctx->h[7] += h;

        in += DEFAULT_BLOCK_SIZE;
    }
}

// ---------------- init / update / final ----------------
static void AlgoInit(AlgoCTX *c)
{
    // initialize context
    c->h[0] = 0x6a09e667UL;
    c->h[1] = 0xbb67ae85UL;
    c->h[2] = 0x3c6ef372UL;
    c->h[3] = 0xa54ff53aUL;
    c->h[4] = 0x510e527fUL;
    c->h[5] = 0x9b05688cUL;
    c->h[6] = 0x1f83d9abUL;
    c->h[7] = 0x5be0cd19UL;
    c->num = 0;
    c->nbits = 0;
    c->mdLen = DEFAULT_HASH_SIZE;
}

static bool AlgoUpdate(AlgoCTX *c, const unsigned char *data, size_t len)
{
    CHECK_AND_RETURN_RET(len != 0, true);

    // update bit count
    uint64_t add = static_cast<uint64_t>(len) * 8ULL;
    c->nbits += add;

    // if buffer has data, try to fill it to 64 bytes
    if (c->num) {
        size_t need = DEFAULT_BLOCK_SIZE - c->num;
        if (len < need) {
            CHECK_AND_RETURN_RET(memcpy_s(c->data + c->num, sizeof(c->data) - c->num, data, len) == EOK, false);
            c->num += static_cast<unsigned int>(len);
            return true;
        } else {
            CHECK_AND_RETURN_RET(memcpy_s(c->data + c->num, sizeof(c->data) - c->num, data, need) == EOK, false);
            // process full block
            AlgoTransform(c, c->data, 1);
            data += need;
            len -= need;
            c->num = 0;
        }
    }

    // process directly from input as many 64-byte blocks as possible
    if (len >= DEFAULT_BLOCK_SIZE) {
        size_t blocks = len / DEFAULT_BLOCK_SIZE;
        // transform directly on input, must be aligned by bytes; we operate on pointer
        AlgoTransform(c, data, blocks);
        size_t consumed = blocks * DEFAULT_BLOCK_SIZE;
        data += consumed;
        len -= consumed;
    }

    // copy remainder into c->data
    if (len) {
        CHECK_AND_RETURN_RET(memcpy_s(c->data, sizeof(c->data), data, len) == EOK, false);
        c->num = static_cast<uint32_t>(len);
    }

    return true;
}

static bool AlgoFinal(unsigned char *md, AlgoCTX* c)
{
    unsigned char tmp[DEFAULT_BLOCK_SIZE];
    // copy remaining bytes into tmp
    if (c->num) {
        CHECK_AND_RETURN_RET(memcpy_s(tmp, sizeof(tmp), c->data, c->num) == EOK, false);
    }

    size_t len = static_cast<size_t>(c->num);
    // append 0x80
    tmp[len++] = 0x80; // padding

    if (len > DEFAULT_PAD_POSITION) {
        // pad with zeros to 64
        CHECK_AND_RETURN_RET(memset_s(tmp + len, sizeof(tmp) - len, 0, DEFAULT_BLOCK_SIZE - len) == EOK, false);
        // process block
        AlgoTransform(c, tmp, 1);
        // reset length
        len = 0;
    }

    // pad zeros until position 56
    if (len < DEFAULT_PAD_POSITION) {
        CHECK_AND_RETURN_RET(memset_s(tmp + len, sizeof(tmp) - len, 0, DEFAULT_PAD_POSITION - len) == EOK, false);
        len = DEFAULT_PAD_POSITION;
    }

    // append bit length as big-endian 64-bit
    unsigned long long bits = c->nbits;
    tmp[56] = static_cast<unsigned char>((bits >> 56) & 0xFFULL);
    tmp[57] = static_cast<unsigned char>((bits >> 48) & 0xFFULL);
    tmp[58] = static_cast<unsigned char>((bits >> 40) & 0xFFULL);
    tmp[59] = static_cast<unsigned char>((bits >> 32) & 0xFFULL);
    tmp[60] = static_cast<unsigned char>((bits >> 24) & 0xFFULL);
    tmp[61] = static_cast<unsigned char>((bits >> 16) & 0xFFULL);
    tmp[62] = static_cast<unsigned char>((bits >> 8) & 0xFFULL);
    tmp[63] = static_cast<unsigned char>(bits & 0xFFULL);

    // final block transform
    AlgoTransform(c, tmp, 1);

    // output digest (big-endian)
    for (int i = 0; i < DEFAULT_NUM_WORDS; ++i) {
        md[BYTES_PER_WORD * i] = static_cast<unsigned char>((c->h[i] >> 24) & 0xFFU);
        md[BYTES_PER_WORD * i + 1] = static_cast<unsigned char>((c->h[i] >> 16) & 0xFFU);
        md[BYTES_PER_WORD * i + 2] = static_cast<unsigned char>((c->h[i] >> 8) & 0xFFU);
        md[BYTES_PER_WORD * i + 3] = static_cast<unsigned char>(c->h[i] & 0xFFU);
    }

    // clear context sensitive data
    memset_s(c, sizeof(*c), 0, sizeof(*c)); // ignore clearing failure, we already have digest

    return true;
}
} // AudioStandard
} // OHOS