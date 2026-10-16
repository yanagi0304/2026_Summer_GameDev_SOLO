#pragma once
// ============================================================================
// MeshSlicer.h  -  DxLib用 メッシュ切断(Slice／スライス) + キャップ(Cap／キャップ)生成
//
// 対象: 動かない(固定の)MV1モデル。ヘッダのみで使える。
//
// 使い方
//   [読み込み時]
//     int model = MV1LoadModel("mannequin.mv1");
//     MV1SetPosition(model, pos);            // 位置・拡縮・回転を先に決めておく
//     auto tris = MeshSlicer::ExtractTriangles(model);        // ワールド座標の三角形を取得
//     auto tex  = MeshSlicer::GetMaterialTextures(model);     // マテリアルごとのテクスチャ
//
//   [切断時]
//     MeshSlicer::Plane pl = MeshSlicer::MakePlane(hitPoint, bladeDir, slashDir);
//     auto res = MeshSlicer::Slice(tris, (int)tex.size(), pl, GetColorU8(200, 30, 30, 255));
//     if (res.sliced) {
//         MV1SetVisible(model, FALSE);       // 元のモデルを消す
//         pieces[0] = res.front;  pieces[1] = res.back;   // 2つの片を保持
//     }
//
//   [毎フレーム]
//     MeshSlicer::Update(pieces[0], dt);  MeshSlicer::Draw(pieces[0], tex);
//     MeshSlicer::Update(pieces[1], dt);  MeshSlicer::Draw(pieces[1], tex);
// ============================================================================

#include "DxLib.h"
#include <vector>
#include <map>
#include <tuple>
#include <utility>
#include <cmath>

namespace MeshSlicer
{
    // ---------------------------------------------------------------- 型
    struct Plane { VECTOR p; VECTOR n; };          // p: 平面上の点, n: 法線(正規化済み)

    struct Tri { VERTEX3D v[3]; int mat; };      // 三角形1枚 + マテリアル番号

    struct Piece                                   // 切断後の片
    {
        std::vector<std::vector<VERTEX3D>> body;   // マテリアルごとの頂点配列(3頂点で1ポリゴン)
        std::vector<VERTEX3D> cap;                 // 断面(キャップ)
        VECTOR pos = VGet(0, 0, 0);             // 片の中心(ワールド座標)
        VECTOR rot = VGet(0, 0, 0);             // 回転(ラジアン)
        VECTOR vel = VGet(0, 0, 0);             // 速度
        VECTOR angVel = VGet(0, 0, 0);             // 角速度
    };

    struct SliceResult
    {
        Piece front;      // 平面の法線側
        Piece back;       // 法線と反対側
        bool  sliced = false;
    };

    // ---------------------------------------------------------------- 取得
    // MV1モデルから三角形を取り出す(ワールド座標)。
    // ※ MV1_REF_* のメンバ名はDxLibのバージョンで異なる場合があるので、
    //    コンパイルエラーが出たらDxDataType.hで確認して、この関数だけ直す。
    inline std::vector<Tri> ExtractTriangles(int model, int frameIndex = -1)
    {
        std::vector<Tri> out;
        MV1SetupReferenceMesh(model, frameIndex, TRUE);
        MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(model, frameIndex, TRUE);

        out.reserve(ref.PolygonNum);
        for (int i = 0; i < ref.PolygonNum; i++)
        {
            const MV1_REF_POLYGON& poly = ref.Polygons[i];
            Tri t;
            t.mat = poly.MaterialIndex;
            for (int k = 0; k < 3; k++)
            {
                const MV1_REF_VERTEX& rv = ref.Vertexs[poly.VIndex[k]];
                VERTEX3D& v = t.v[k];
                v.pos = rv.Position;
                v.norm = rv.Normal;
                v.dif = GetColorU8(255, 255, 255, 255);
                v.spc = GetColorU8(0, 0, 0, 0);
                v.u = rv.TexCoord[0].u;
                v.v = rv.TexCoord[0].v;
                v.su = 0.0f;
                v.sv = 0.0f;
            }
            out.push_back(t);
        }
        return out;
    }

    // マテリアルごとのテクスチャハンドルを取得
    inline std::vector<int> GetMaterialTextures(int model)
    {
        int n = MV1GetMaterialNum(model);
        std::vector<int> v(n, DX_NONE_GRAPH);
        for (int i = 0; i < n; i++)
        {
            int ti = MV1GetMaterialDifMapTexture(model, i);
            if (ti >= 0) v[i] = MV1GetTextureGraphHandle(model, ti);
        }
        return v;
    }

    // ---------------------------------------------------------------- 切断平面
    // 剣の刃の向き(bladeDir)と、振った方向(slashDir)から切断平面を作る
    inline Plane MakePlane(const VECTOR& hitPoint, const VECTOR& bladeDir, const VECTOR& slashDir)
    {
        Plane pl;
        pl.p = hitPoint;
        pl.n = VNorm(VCross(bladeDir, slashDir));
        return pl;
    }

    // ---------------------------------------------------------------- 内部処理
    namespace detail
    {
        inline float Dist(const Plane& pl, const VECTOR& x)
        {
            return VDot(pl.n, VSub(x, pl.p));          // 正: 表側 / 負: 裏側
        }

        inline bool Less(const VECTOR& a, const VECTOR& b)
        {
            if (a.x != b.x) return a.x < b.x;
            if (a.y != b.y) return a.y < b.y;
            return a.z < b.z;
        }

        // 辺 a-b と平面の交点。
        // 隣り合う三角形が「まったく同じ座標」の交点を得られるように、
        // 端点の順番を座標で固定してから計算する(キャップのループ接続に必要)。
        inline VERTEX3D Intersect(const Plane& pl, const VERTEX3D& a, const VERTEX3D& b)
        {
            const VERTEX3D* p = &a;
            const VERTEX3D* q = &b;
            if (Less(b.pos, a.pos)) std::swap(p, q);

            float dp = Dist(pl, p->pos);
            float dq = Dist(pl, q->pos);
            float t = dp / (dp - dq);                 // 両端は必ず反対側なので分母は0にならない

            VERTEX3D r = *p;
            r.pos = VAdd(p->pos, VScale(VSub(q->pos, p->pos), t));
            r.norm = VNorm(VAdd(VScale(p->norm, 1.0f - t), VScale(q->norm, t)));
            r.u = p->u + (q->u - p->u) * t;
            r.v = p->v + (q->v - p->v) * t;
            return r;
        }

        struct Seg { VECTOR a, b; };                   // 切断面上の線分
        using Key = std::tuple<long long, long long, long long>;

        inline Key MakeKey(const VECTOR& v, float cell)
        {
            return Key((long long)std::llround(v.x / cell),
                (long long)std::llround(v.y / cell),
                (long long)std::llround(v.z / cell));
        }

        inline void AddTri(Piece& pc, int mat, const VERTEX3D& a, const VERTEX3D& b, const VERTEX3D& c)
        {
            auto& dst = pc.body[mat];
            dst.push_back(a); dst.push_back(b); dst.push_back(c);
        }

        inline VERTEX3D MakeCapVertex(const VECTOR& p, const VECTOR& n, COLOR_U8 col)
        {
            VERTEX3D v = {};
            v.pos = p;
            v.norm = n;
            v.dif = col;
            v.spc = GetColorU8(0, 0, 0, 0);
            return v;
        }

        // 線分をつなげて「輪(ループ)」にする。両脚を同時に切った場合などは輪が複数できる。
        inline std::vector<std::vector<VECTOR>> BuildLoops(const std::vector<Seg>& segs, float weld)
        {
            std::map<Key, std::vector<int>> adj;
            for (int i = 0; i < (int)segs.size(); i++)
            {
                adj[MakeKey(segs[i].a, weld)].push_back(i);
                adj[MakeKey(segs[i].b, weld)].push_back(i);
            }

            std::vector<char> used(segs.size(), 0);
            std::vector<std::vector<VECTOR>> loops;

            for (size_t s = 0; s < segs.size(); s++)
            {
                if (used[s]) continue;
                used[s] = 1;

                std::vector<VECTOR> loop;
                loop.push_back(segs[s].a);
                const Key startKey = MakeKey(segs[s].a, weld);
                VECTOR cur = segs[s].b;

                for (;;)
                {
                    Key ck = MakeKey(cur, weld);
                    if (ck == startKey) break;         // 輪が閉じた
                    loop.push_back(cur);

                    int next = -1;
                    for (int idx : adj[ck]) { if (!used[idx]) { next = idx; break; } }
                    if (next < 0) break;               // 閉じない場合はここで終了
                    used[next] = 1;

                    const Seg& sg = segs[next];
                    cur = (MakeKey(sg.a, weld) == ck) ? sg.b : sg.a;
                }
                if (loop.size() >= 3) loops.push_back(loop);
            }
            return loops;
        }

        // 片の頂点を中心基準(ローカル座標)に変換して、posに中心を入れる
        inline void Finalize(Piece& pc)
        {
            VECTOR sum = VGet(0, 0, 0);
            int cnt = 0;
            for (auto& vec : pc.body)
                for (auto& v : vec) { sum = VAdd(sum, v.pos); cnt++; }
            for (auto& v : pc.cap) { sum = VAdd(sum, v.pos); cnt++; }
            if (cnt == 0) return;

            VECTOR c = VScale(sum, 1.0f / cnt);
            for (auto& vec : pc.body)
                for (auto& v : vec) v.pos = VSub(v.pos, c);
            for (auto& v : pc.cap)  v.pos = VSub(v.pos, c);
            pc.pos = c;
        }
    } // namespace detail

    // ---------------------------------------------------------------- 切断本体
    // tris        : ExtractTriangles の結果
    // materialNum : マテリアル数(GetMaterialTextures の要素数)
    // capColor    : 断面の色
    // pushPower   : 切断後に2つの片が離れる速さ
    // weld        : 交点を同一点とみなす距離(モデルの単位に合わせる)
    inline SliceResult Slice(const std::vector<Tri>& tris, int materialNum, const Plane& pl,
        COLOR_U8 capColor, float pushPower = 30.0f, float weld = 0.01f)
    {
        using namespace detail;

        SliceResult res;
        if (materialNum < 1) materialNum = 1;
        res.front.body.resize(materialNum);
        res.back.body.resize(materialNum);

        std::vector<Seg> segs;

        for (const Tri& t : tris)
        {
            int mat = t.mat;
            if (mat < 0 || mat >= materialNum) mat = 0;

            bool side[3];                               // true: 表側
            for (int k = 0; k < 3; k++) side[k] = Dist(pl, t.v[k].pos) >= 0.0f;
            int nFront = (int)side[0] + (int)side[1] + (int)side[2];

            // 1) 全頂点が同じ側 → そのまま振り分け
            if (nFront == 3) { AddTri(res.front, mat, t.v[0], t.v[1], t.v[2]); continue; }
            if (nFront == 0) { AddTri(res.back, mat, t.v[0], t.v[1], t.v[2]); continue; }

            // 2) 平面をまたぐ → 1つだけ反対側にある頂点(lone)を探す
            int lone = 0;
            for (int k = 0; k < 3; k++)
                if (side[k] != side[(k + 1) % 3] && side[k] != side[(k + 2) % 3]) lone = k;

            const int j = (lone + 1) % 3;
            const int m = (lone + 2) % 3;
            const VERTEX3D& Vi = t.v[lone];
            const VERTEX3D& Vj = t.v[j];
            const VERTEX3D& Vm = t.v[m];

            VERTEX3D Pij = Intersect(pl, Vi, Vj);       // 辺 i-j の交点
            VERTEX3D Pmi = Intersect(pl, Vm, Vi);       // 辺 m-i の交点

            Piece& loneSide = side[lone] ? res.front : res.back;
            Piece& otherSide = side[lone] ? res.back : res.front;

            AddTri(loneSide, mat, Vi, Pij, Pmi);      // lone側: 三角形1枚
            AddTri(otherSide, mat, Pij, Vj, Vm);       // 反対側: 四角形 = 三角形2枚
            AddTri(otherSide, mat, Pij, Vm, Pmi);

            segs.push_back({ Pij.pos, Pmi.pos });       // キャップ用の線分
        }

        // 片側が空 = 平面がモデルに当たっていない
        auto empty = [](const Piece& pc) {
            for (auto& v : pc.body) if (!v.empty()) return false;
            return true;
            };
        if (empty(res.front) || empty(res.back)) return res;

        // 3) キャップ生成: 線分を輪につなぎ、輪ごとに重心から扇状(Triangle Fan)に埋める
        //    ※ 凸(へこみのない)に近い輪向け。大きくへこんだ輪では重なる場合がある。
        auto loops = BuildLoops(segs, weld);
        const VECTOR nFrontCap = VScale(pl.n, -1.0f);   // 表側の片の断面は、反対向き
        const VECTOR nBackCap = pl.n;

        for (auto& loop : loops)
        {
            VECTOR c = VGet(0, 0, 0);
            for (auto& p : loop) c = VAdd(c, p);
            c = VScale(c, 1.0f / (float)loop.size());

            for (size_t i = 0; i < loop.size(); i++)
            {
                const VECTOR& p0 = loop[i];
                const VECTOR& p1 = loop[(i + 1) % loop.size()];

                res.front.cap.push_back(MakeCapVertex(c, nFrontCap, capColor));
                res.front.cap.push_back(MakeCapVertex(p0, nFrontCap, capColor));
                res.front.cap.push_back(MakeCapVertex(p1, nFrontCap, capColor));

                res.back.cap.push_back(MakeCapVertex(c, nBackCap, capColor));
                res.back.cap.push_back(MakeCapVertex(p0, nBackCap, capColor));
                res.back.cap.push_back(MakeCapVertex(p1, nBackCap, capColor));
            }
        }

        // 4) 片の中心を求めて、頂点を中心基準にする
        Finalize(res.front);
        Finalize(res.back);

        // 5) 離れる方向に初速を与える(法線方向 + 少し回転)
        res.front.vel = VScale(pl.n, pushPower);
        res.back.vel = VScale(pl.n, -pushPower);
        res.front.angVel = VGet(0.0f, 0.0f, 1.5f);
        res.back.angVel = VGet(0.0f, 0.0f, -1.5f);

        res.sliced = true;
        return res;
    }

    // ---------------------------------------------------------------- 更新・描画
    // 簡易物理: 重力 + 地面で止まる。gravityはモデルの単位に合わせる。
    inline void Update(Piece& pc, float dt, float groundY = 0.0f, float gravity = -980.0f)
    {
        pc.vel.y += gravity * dt;
        pc.pos = VAdd(pc.pos, VScale(pc.vel, dt));
        pc.rot = VAdd(pc.rot, VScale(pc.angVel, dt));

        if (pc.pos.y < groundY)
        {
            pc.pos.y = groundY;
            pc.vel.y = 0.0f;
            pc.vel.x *= 0.9f;  pc.vel.z *= 0.9f;
            pc.angVel = VScale(pc.angVel, 0.9f);
        }
    }

    inline void Draw(const Piece& pc, const std::vector<int>& tex)
    {
        MATRIX rot = MMult(MMult(MGetRotX(pc.rot.x), MGetRotY(pc.rot.y)), MGetRotZ(pc.rot.z));
        MATRIX world = MMult(rot, MGetTranslate(pc.pos));
        SetTransformToWorld(&world);

        for (size_t m = 0; m < pc.body.size(); m++)
        {
            if (pc.body[m].empty()) continue;
            int gr = (m < tex.size()) ? tex[m] : DX_NONE_GRAPH;
            DrawPolygon3D(const_cast<VERTEX3D*>(pc.body[m].data()),
                (int)(pc.body[m].size() / 3), gr, TRUE);
        }

        if (!pc.cap.empty())
        {
            int oldCull = GetUseBackCulling();
            SetUseBackCulling(DX_CULLING_NONE);         // 断面は両面描画(ポリゴンの向きを気にしなくてよい)
            DrawPolygon3D(const_cast<VERTEX3D*>(pc.cap.data()),
                (int)(pc.cap.size() / 3), DX_NONE_GRAPH, FALSE);
            SetUseBackCulling(oldCull);
        }

        MATRIX ident = MGetIdent();
        SetTransformToWorld(&ident);
    }

} // namespace MeshSlicer
