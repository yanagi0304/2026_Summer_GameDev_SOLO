#pragma once
#include <Eigen/Core>
#include <igl/slice.h>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "../../../Utility/Utility.h"
#include "../../../Common/Vector3.h"


namespace fs = std::filesystem;


class MeshSlicer
{
public:

	MeshSlicer();
	~MeshSlicer() = default;

    struct SlicedResult
    {
        Eigen::MatrixXd vA; // パーツAの頂点
        Eigen::MatrixXi fA; // パーツAのポリゴン
        Eigen::MatrixXd vB; // パーツBの頂点
        Eigen::MatrixXi fB; // パーツBのポリゴン
    };


    // モデルのデータを読み込む関数
    bool LoadObj(const std::string& filePath)
    {
        // フォルダ内を走査
        for (const auto& entry : fs::directory_iterator(filePath)) {
            // ファイルじゃなければスキップ
            if (!entry.is_regular_file()) { continue; }

            // 拡張子取得
            const fs::path& path = entry.path();

            // .csvじゃなければスキップ
            if (path.extension() != ".txt") { continue; }

            // ファイル名のみ取得（拡張子なし）
            std::string fileName = path.stem().string();

            // Txt読み込み
            LoadTxtFile(path.string(), fileName);
        }
    }

    // 刀の平面でメッシュを切断する関数
    // (引数: 読み込み済みのモデル名, 刀の中心位置, 刀の刃の向き[法線])
    SlicedResult SliceMesh(const std::string& modelName, const Vector3& planePoint, const Vector3& planeNormal)
    {
        SlicedResult result;

        // 倉庫に名前があるか安全チェック
        if (modelVerticesMap_.find(modelName) == modelVerticesMap_.end()) {
            std::cerr << "エラー: 倉庫にそのモデル名がありません: " << modelName << std::endl;
            return result;
        }

        // 倉庫から元の巨大なデータを引っ張り出す
        const Eigen::MatrixXd& V = modelVerticesMap_[modelName];
        const Eigen::MatrixXi& F = modelFacesMap_[modelName];

        int vertexCount = V.rows();
        int triangleCount = F.rows();


        // すべての頂点と「刀の平面」の位置関係（距離）を計算する
        Eigen::VectorXd S(vertexCount);

        for (int i = 0; i < vertexCount; ++i) {
            // (現在の頂点座標 - 刀の中心点)
            double dx = V(i, 0) - planePoint.x;
            double dy = V(i, 1) - planePoint.y;
            double dz = V(i, 2) - planePoint.z;

            double distance = dx * planeNormal.x + dy * planeNormal.y + dz * planeNormal.z;

            S(i) = distance; // プラスなら表、マイナスなら裏
        }

        // 表側（パーツA）と裏側（パーツB）のポリゴンを仕分ける
        std::vector<int> keepIndicesA;
        std::vector<int> keepIndicesB;

        for (int i = 0; i < triangleCount; ++i) {
            // 三角形の1つ目の頂点番号を取得
            int idx0 = F(i, 0);

            // その頂点がプラス側にあればパーツA、マイナス側ならパーツBに仕分ける
            if (S(idx0) >= 0.0) {
                keepIndicesA.push_back(i); // パーツAに残す三角形の番号
            }
            else {
                keepIndicesB.push_back(i); // パーツBに残す三角形の番号
            }
        }

    
        // パーツAの抽出
        if (!keepIndicesA.empty()) {
            Eigen::VectorXi RA(keepIndicesA.size());
            for (size_t i = 0; i < keepIndicesA.size(); ++i) RA(i) = keepIndicesA[i];

            // 行列 F から指定した行だけをスライスして fA を作る
            igl::slice(F, RA, 1, result.fA);
            result.vA = V; // 頂点データはそのまま引き継ぐ
        }

        // パーツBの抽出
        if (!keepIndicesB.empty()) {
            Eigen::VectorXi RB(keepIndicesB.size());
            for (size_t i = 0; i < keepIndicesB.size(); ++i) RB(i) = keepIndicesB[i];

            // 行列 F から指定した行だけをスライスして fB を作る
            igl::slice(F, RB, 1, result.fB);
            result.vB = V; // 頂点データはそのまま引き継ぐ
        }

        return result;
    }



private:

    void LoadTxtFile(const std::string& filePath, const std::string& fileName)
    {
        std::ifstream ifs(filePath);
        if (!ifs) {
            std::cerr << "txtファイルを開けませんでした : " << filePath << std::endl;
            return;
        }

        // 読み込んだデータを一時的に溜める
        std::vector<std::vector<double>> tempVertices;
        std::vector<std::vector<int>> tempFaces;

        std::string line;
        while (getline(ifs, line)) {
            if (line.empty()) { continue; }

            std::stringstream ss(line);
            std::string type;
            ss >> type; // 行の先頭の文字（ v や f などの識別子）を取得

            // 必要なデータ種別（ v か f 
            if (type == "v") {
                double x, y, z;
                ss >> x >> y >> z;
                tempVertices.push_back({ x, y, z });
            }
            else if (type == "f") {
                // 1行に何個の頂点札があるか分からないので、一旦バラバラに配列に入れる
                std::vector<std::string> faceTokens;
                std::string token;
                while (ss >> token) {
                    faceTokens.push_back(token);
                }

                // 頂点が3つの場合
                if (faceTokens.size() == 3) {
                    int idx1 = std::stoi(faceTokens[0].substr(0, faceTokens[0].find('/'))) - 1;
                    int idx2 = std::stoi(faceTokens[1].substr(0, faceTokens[1].find('/'))) - 1;
                    int idx3 = std::stoi(faceTokens[2].substr(0, faceTokens[2].find('/'))) - 1;
                    tempFaces.push_back({ idx1, idx2, idx3 });
                }
                // 頂点が4つの場合
                else if (faceTokens.size() == 4) {
                    int idx1 = std::stoi(faceTokens[0].substr(0, faceTokens[0].find('/'))) - 1;
                    int idx2 = std::stoi(faceTokens[1].substr(0, faceTokens[1].find('/'))) - 1;
                    int idx3 = std::stoi(faceTokens[2].substr(0, faceTokens[2].find('/'))) - 1;
                    int idx4 = std::stoi(faceTokens[3].substr(0, faceTokens[3].find('/'))) - 1;

                    // 1つ目の三角形 (A, B, C)
                    tempFaces.push_back({ idx1, idx2, idx3 });
                    // 2つ目の三角形 (A, C, D)
                    tempFaces.push_back({ idx1, idx3, idx4 });
                }
            }
            // それ以外の行（#, mtllib, o など）はスキップ
        }

        // 読み込み終わったデータをEigen 行列に変換して保存
        Eigen::MatrixXd V(tempVertices.size(), 3);
        for (size_t i = 0; i < tempVertices.size(); ++i) {
            V(i, 0) = tempVertices[i][0];
            V(i, 1) = tempVertices[i][1];
            V(i, 2) = tempVertices[i][2];
        }

        Eigen::MatrixXi F(tempFaces.size(), 3);
        for (size_t i = 0; i < tempFaces.size(); ++i) {
            F(i, 0) = tempFaces[i][0];
            F(i, 1) = tempFaces[i][1];
            F(i, 2) = tempFaces[i][2];
        }

        // ファイル名をキーにして、Eigenの行列をマップに保存する
        modelVerticesMap_[fileName] = V;
        modelFacesMap_[fileName] = F;

        std::cout << "モデル [" << fileName << "] の読み込みに成功！ 頂点数: " << V.rows() << " 三角形数: " << F.rows() << std::endl;
    }

private:
    
    // ファイル名をキーにして、それぞれの Eigen 行列を保管するマップ
    std::map<std::string, Eigen::MatrixXd> modelVerticesMap_;
    std::map<std::string, Eigen::MatrixXi> modelFacesMap_;

};

