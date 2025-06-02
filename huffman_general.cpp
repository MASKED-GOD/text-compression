#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <string>

using namespace std;

struct Node {
    unsigned char ch;
    int freq;
    Node *left, *right;
    Node(unsigned char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

unordered_map<unsigned char, string> huffmanCodes;
unordered_map<string, unsigned char> reverseCodes;

void buildCodes(Node* root, string str) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanCodes[root->ch] = str;
        reverseCodes[str] = root->ch;
    }
    buildCodes(root->left, str + "0");
    buildCodes(root->right, str + "1");
}

void saveTree(ofstream& out, Node* root) {
    if (!root) return;
    if (!root->left && !root->right) {
        out.put('1');
        out.put(root->ch);
        return;
    }
    out.put('0');
    saveTree(out, root->left);
    saveTree(out, root->right);
}

Node* loadTree(ifstream& in) {
    char flag = in.get();
    if (flag == '1') {
        unsigned char c = in.get();
        return new Node(c, 0);
    }
    Node* left = loadTree(in);
    Node* right = loadTree(in);
    Node* root = new Node(0, 0);
    root->left = left;
    root->right = right;
    return root;
}

void compress(const string& inputPath, const string& outputPath) {
    ifstream in(inputPath, ios::binary);
    if (!in) {
        cout << "Failed to open input file.\n";
        return;
    }
    vector<unsigned char> data((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    unordered_map<unsigned char, int> freq;
    for (unsigned char c : data) freq[c]++;

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& p : freq) pq.push(new Node(p.first, p.second));
    while (pq.size() > 1) {
        Node* a = pq.top(); pq.pop();
        Node* b = pq.top(); pq.pop();
        Node* merged = new Node(0, a->freq + b->freq);
        merged->left = a;
        merged->right = b;
        pq.push(merged);
    }
    Node* root = pq.top();
    buildCodes(root, "");

    string encoded;
    for (unsigned char c : data) encoded += huffmanCodes[c];
    while (encoded.size() % 8 != 0) encoded += '0'; // Padding

    ofstream out(outputPath, ios::binary);
    saveTree(out, root);
    out.put('|'); // Separator
    for (size_t i = 0; i < encoded.size(); i += 8) {
        bitset<8> bits(encoded.substr(i, 8));
        out.put(static_cast<unsigned char>(bits.to_ulong()));
    }
    out.close();
    cout << "Compressed to: " << outputPath << endl;
}

void decompress(const string& inputPath, const string& outputPath) {
    ifstream in(inputPath, ios::binary);
    if (!in) {
        cout << "Failed to open compressed file.\n";
        return;
    }
    Node* root = loadTree(in);
    if (in.get() != '|') {
        cout << "Invalid file format.\n";
        return;
    }
    string bitStr;
    char ch;
    while (in.get(ch)) {
        bitset<8> bits((unsigned char)ch);
        bitStr += bits.to_string();
    }
    in.close();

    ofstream out(outputPath, ios::binary);
    Node* node = root;
    for (char bit : bitStr) {
        node = (bit == '0') ? node->left : node->right;
        if (!node->left && !node->right) {
            out.put(node->ch);
            node = root;
        }
    }
    out.close();
    cout << "Decompressed to: " << outputPath << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage:\n";
        cout << "  Compress:   ./huffman_general compress <input_file> <output_file>\n";
        cout << "  Decompress: ./huffman_general decompress <input_file> <output_file>\n";
        return 1;
    }
    string mode = argv[1];
    string inPath = argv[2];
    string outPath = argv[3];
    if (mode == "compress")
        compress(inPath, outPath);
    else if (mode == "decompress")
        decompress(inPath, outPath);
    else
        cout << "Unknown mode. Use 'compress' or 'decompress'.\n";
    return 0;
} 