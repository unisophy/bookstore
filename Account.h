#ifndef NI_DIE_H_ACCOUNT_H
#define NI_DIE_H_ACCOUNT_H
#include "UnrolledLinkedList.h"
#include "error.h"

struct account {
    int numOFlogin = 0;
    std::string ID;
    std::string Name;
    std::string Password;
    int Privilege;
    account() = default;
    account(const std::string &id, const std::string &n,
         const std::string &p, const int &pr) {
        ID = id;
        Name = n;
        Password = p;
        Privilege = pr;
        numOFlogin = 1;
    }
    void ChangePassword(const std::string &cp, const std::string &ep) {
        if (Password != cp) {throw error();}
        Password = ep;
    }
    void ChangePassword(const std::string &ep) {
        Password = ep;
    }
};

class AccountSystem {
private:
    int numOFaccount;
    UnrolledLinkedList IDIndex;
    std::fstream iofile;
    const std::string filename = "Account";
    account now_user;

    int find(const std::string &id, account &a) {
        String _id(id);
        static bool j = false;
        static std::vector<int>vec;
        vec.clear();
        IDIndex.Find(id, j,vec);
        if (!j) {return -1;}
        iofile.seekg(4 + vec[0] * sizeof(account));
        return vec[0];
    }

    void del(account &a, const int &position) {
        String _id(a.ID);
        element ele(_id, position);
        IDIndex.Delete(ele);
    }

    bool Judge(const std::string &id) {
        static bool j = false;
        static std::vector<int>vec;
        String _id(id);
        vec.clear();
        IDIndex.Find(_id, j, vec);
        return j;
    }

public:
    std::stack<int>s;
    AccountSystem() {
        iofile.open(filename);
        if (!iofile) {
            static account a;
            numOFaccount = -1;
            iofile.open(filename, std::fstream::out);
            iofile.close();
            iofile.open(filename);
            iofile.seekp(0);
            iofile.write(reinterpret_cast<char *>(&numOFaccount), sizeof(numOFaccount));
            iofile.seekp(4);
            iofile.write(reinterpret_cast<char *>(&a), sizeof(a));
        }
        else {
            iofile.seekg(0);
            iofile.read(reinterpret_cast<char *>(&numOFaccount), sizeof(numOFaccount));
        }
        while (!s.empty()) {
            s.pop();
        }
    }

    ~AccountSystem() {
        iofile.seekp(0);
        iofile.write(reinterpret_cast<char *>(&numOFaccount), sizeof(numOFaccount));
        iofile.close();
        while (!s.empty()) {
            s.pop();
        }
    }

    void initialize() {
        std::string manager_name = "root";
        account a(manager_name, manager_name, "sjtu", 7);
        numOFaccount += 1;
        iofile.seekp(4 + numOFaccount * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a), sizeof(account));
        element ele(manager_name, numOFaccount);
        IDIndex.Insert(ele);
    }

    void su(const std::string &id, const std::string &p) {
        account a;
        int index = find(id, a);
        if (index == -1) {
            throw error("No user matched");
        }
        if (a.Password != p) {
            throw error("Wrong Password");
        }
        s.push(index);
        now_user = a;
        a.numOFlogin += 1;
        iofile.seekp(4 + index * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a.numOFlogin), sizeof(int));
    }

    void su(const std::string &id) {
        account a;
        int index = find(id, a);
        if (index == -1) {
            throw error("No user matched");
        }
        s.push(index);
        now_user = a;
        iofile.seekp(4 + index * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a.numOFlogin), sizeof(int));
    }

    void logout() {
        if (s.empty()) {throw error("No user is logged in");}
        int index = s.top();
        iofile.seekp(4 + index * sizeof(account));
        now_user.numOFlogin -= 1;
        iofile.write(reinterpret_cast<char *>(&now_user.numOFlogin), sizeof(int));
        s.pop();
        if (!s.empty()) {
            index = s.top();
            iofile.seekg(4 + index * sizeof(account));
            iofile.read(reinterpret_cast<char *>(&now_user), sizeof(now_user));
        }
    }

    void Register(const std::string &id, const std::string &n, const std::string &p) {
        if (!Judge(id)) {throw error("Repeated User ID");}
        int pr = 1;
        account a(id, n, p, pr);
        numOFaccount += 1;
        iofile.seekp(4 + numOFaccount * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a), sizeof(a));
        element ele(id,numOFaccount);
        IDIndex.Insert(ele);
    }

    void passwd(const std::string &id, const std::string &cp, const std::string &p) {
        if (s.empty()) {throw error("No user logged in");}
        account a;
        int index = find(id,a);
        if (index == -1) {throw error("No such user");}
        a.ChangePassword(cp, p);
        iofile.seekp(4 + index * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a), sizeof(account));
    }

    void passwd(const std::string &id, const std::string &p) {
        if (s.empty()) {throw error("No user logged in");}
        if (now_user.Privilege < 7) {throw error("Missing current password");}
        account a;
        int index = find(id, a);
        if (index == -1) throw error("No such user");
        a.ChangePassword(p);
        iofile.seekp(4 + index * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a), sizeof(account));
    }

    void useradd(const std::string &id, const std::string &n, const std::string &p, const int &pr) {
        if (s.empty()) {throw error("Not logged in");}
        if (now_user.Privilege < 3) {throw error("No right to add user");}
        if (pr >= now_user.Privilege) {throw error("No right to add user");}
        if (Judge(id)) {throw error("Repeated User ID");}
        account a(id, n, p, pr);
        numOFaccount += 1;
        iofile.seekp(4 + numOFaccount * sizeof(account));
        iofile.write(reinterpret_cast<char *>(&a), sizeof(account));
        element ele(id,numOFaccount);
        IDIndex.Insert(ele);
    }

    void Delete(const std::string &id) {
        if (s.empty()) {throw error("Not logged in");}
        if (now_user.Privilege < 7) {throw error("No right to delete user");}
        account a;
        int index = find(id, a);
        if (index == -1) throw error("No such user");
        if (a.numOFlogin != 0) {throw error("This account is logged in");}
        del(a,index);
    }
};
#endif //NI_DIE_H_ACCOUNT_H
