#ifndef NI_DIE_H_UNROLLEDLINKEDLIST_H
#define NI_DIE_H_UNROLLEDLINKEDLIST_H

#include <bits/stdc++.h>
#include <fstream>

class String {
private:
    char s[65];
public:
    String() = default;
    String(const char* p) {
        int length;
        length = strlen(p);
        for (int i = 0; i < length; ++i) {
            s[i] = p[i];
        }
        s[length] = '\0';
    }
    friend bool operator<(const String &a, const String &b) {
        return (strcmp(a.s, b.s) < 0);
    }
    friend bool operator==(const String &a, const String &b) {
        return (strcmp(a.s, b.s) == 0);
    }
    friend bool operator!=(const String &a, const String &b) {
        return (!(a == b));
    }
    friend bool operator<=(const String &a, const String &b) {
        return (a < b) || (a == b);
    }
    friend std::ostream& operator<<(std::ostream &os, const String &a) {
        os << a.s;
        return os;
    }
};

struct element {
    String str;
    int value;
    element() = default;
    element(const String &a, const int &b) {
        str = a;
        value = b;
    }
    friend bool operator==(const element &a, const element &b) {
        if (a.str != b.str) {return false;}
        return a.value == b.value;
    }
    friend bool operator<(const element &a, const element &b) {
        if (a.str != b.str) {return (a.str < b.str);}
        return a.value < b.value;
    }
    friend bool operator>(const element &a, const element &b) {
        if (a.str == b.str) {return !(a.str < b.str);}
        return a.value > b.value;
    }
    friend bool operator<=(const element &a, const element &b) {
        return (!(a > b));
    }
    friend bool operator>=(const element &a, const element &b) {
        return (!(a < b));
    }
};

struct block {
    element minele;
    element maxele;
    int next = -1;
    int size = 0;
    element e[2001];
};

class UnrolledLinkedList {
private:
    int maxsize = 1000;
    int minsize = 500;
    int numOFblock = -1;
    std::fstream iofile;
    const std::string filename = "UnrolledLinkedList";
    void readout(int p, block &b) {
        iofile.seekg(p * sizeof(block) + sizeof(int));
        iofile.read(reinterpret_cast<char *>(&b), sizeof(b));
    }

    void readoutApart (int p, block &b) {
        iofile.seekg(p * sizeof(block) + sizeof(int));
        iofile.read(reinterpret_cast<char *>(&(b.minele)), sizeof(b.minele));
        iofile.read(reinterpret_cast<char *>(&(b.maxele)), sizeof(b.maxele));
        iofile.read(reinterpret_cast<char *>(&(b.next)), sizeof(b.next));
        iofile.read(reinterpret_cast<char *>(&(b.size)), sizeof(b.size));
    }

    void writein(int p, block &b) {
        iofile.seekp(p * sizeof(block) + sizeof(int));
        iofile.write(reinterpret_cast<char *>(&b), sizeof(b));
    }
    void Break(int p, block &b) {
        numOFblock += 1;
        block a;
        for (int i = minsize; i < b.size; i++) {
            a.e[i - minsize] = b.e[i];
            a.size += 1;
            b.size -= 1;
        }
        a.next = b.next;
        b.next = numOFblock;
        b.minele = b.e[0];
        a.minele = a.e[0];
        b.maxele = b.e[b.size - 1];
        a.maxele = a.e[a.size - 1];
        writein(p,b);
        writein(numOFblock,a);
    }

    void Debreak(int p, block &b) {
        if (b.next == -1) {
            writein(p,b);
            return;
        }
        block a;
        readout(b.next,a);
        if (b.size + a.size <= maxsize) {
            for (int i = b.size; i < b.size + a.size; i++) {
                b.e[i] = a.e[i - b.size];
            }
            b.size = a.size + b.size;
            b.next = a.next;
            b.minele = b.e[0];
            b.maxele = b.e[b.size - 1];
        }
        writein(p, b);
    }

    void InsertAnElement(int p,const element &e) {
        block b;
        if (numOFblock == -1) {
            numOFblock += 1;
            b.size += 1;
            b.maxele = b.minele = e;
            b.e[0] = e;
            writein(0,b);return;
        }

        readout(p,b);
        int index = -1;
        for (int i = 0; i < b.size; i++) {
            if (e < b.e[i]) {index = i;break;}
            if (e == b.e[i]) {return;}
        }

        if (index == -1) {
            b.e[b.size] = e;
            b.size += 1;
        } else {
            for (int i = b.size; i > index; i--) {
                b.e[i] = b.e[i - 1];
            }
            b.e[index] = e;
            b.size += 1;
        }

        if (b.size > maxsize) {
            Break(p,b);
        } else {
            b.minele = b.e[0];
            b.maxele = b.e[b.size - 1];
            writein(p,b);
        }
    }

    void deleteAnElement(int p,const element &e) {
        block a;
        readout(p,a);
        for (int i = 0 ; i < a.size; i++) {
            if (a.e[i] > e) {break;}
            if (a.e[i] == e) {
                for (int j = i; j < a.size - 1; j++) {
                    a.e[j] = a.e[j + 1];
                }
                a.size -= 1;
                if (a.size == 0) {writein(p, a); break;}
                a.minele = a.e[0];
                a.maxele = a.e[a.size - 1];
                if (a.size < minsize) {
                    Debreak(p,a);
                } else {
                    writein(p,a);
                }
                break;
            }
        }
    }

public:

    UnrolledLinkedList() {
        iofile.open(filename);
        if (!iofile) {
            block b;
            iofile.open(filename, std::fstream::out);
            iofile.close();
            iofile.open(filename);
            iofile.seekp(0);
            iofile.write(reinterpret_cast<char *>(&numOFblock), sizeof(numOFblock));
            iofile.seekp(4);
            iofile.write(reinterpret_cast<char *>(&b), sizeof(b));
        }
        else {
            iofile.seekg(0);
            iofile.read(reinterpret_cast<char *>(&numOFblock), sizeof(numOFblock));
        }
    }

    ~UnrolledLinkedList() {
        iofile.seekp(0);
        iofile.write(reinterpret_cast<char *>(&numOFblock), sizeof(numOFblock));
        iofile.close();
    }

    void Insert (const element &e) {
        block b;
        int now_position = 0;
        readoutApart(0,b);
        if (b.size == 0 || e < b.minele) {
            InsertAnElement(0,e);
            return;
        }
        while(true) {
            if (b.next == -1) {break;}
            block a;
            readoutApart(b.next,a);
            if (e >= b.minele && e <= a.minele) {
                if (e == a.minele) return;
                InsertAnElement(now_position, e);
                return;
            }
            now_position = a.next;
            b = a;
        }
        InsertAnElement(now_position,e);
    }

    void Delete(const element &e) {
        block b;
        int now_position = 0;
        while(true) {
            if (now_position == -1) {break;}
            readoutApart(now_position,b);
            if (b.size == 0) {
                now_position = b.next;
                continue;
            } else if (e >= b.minele && e <= b.maxele) {
                deleteAnElement(now_position, e);
                break;
            } else if (e < b.minele) {break;}
            now_position = b.next;
        }
    }

    void Find(const String &index) {
        block b;
        int now_position = 0;
        bool judge = false;
        while(true) {
            if (now_position == -1) {break;}
            readoutApart(now_position,b);
            if (b.size == 0) {
                now_position = b.next;
                continue;
            } else if (b.minele.str <= index && index <= b.maxele.str) {
                readout(now_position,b);
                for (int i = 0; i < b.size; ++i) {
                    if (b.e[i].str == index) {
                        judge = true;
                        std::cout << b.e[i].value << " ";
                    }
                }
            } else if (index < b.minele.str) {
                break;
            }
        }
        if (!judge) {
            std::cout << "null\n";
        }
        else {
            std::cout << "\n";
        }
    }

};

int main() {
    UnrolledLinkedList jzx;
    int n;
    std::string operation;
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        std::cin >> operation;
        int value;
        char index[65];
        if (operation == "insert") {
            scanf("%s", index);
            std::cin >> value;
            String s(index);
            element e(s, value);
            jzx.Insert(e);
        } else if (operation == "delete") {
            scanf("%s", index);
            std::cin >> value;
            String s(index);
            element e(s, value);
            jzx.Delete(e);
        } else if (operation == "find") {
            scanf("%s", index);
            String s(index);
            jzx.Find(index);
        }
    }
}
#endif //NI_DIE_H_UNROLLEDLINKEDLIST_H
