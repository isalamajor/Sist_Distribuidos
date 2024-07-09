typedef double double_vector<32>;

struct respuesta {
    string value1<256>;
    double_vector V_value2;
    int todo_ok;
};

program CLAVES_PROG {
    version CLAVES_VERSION {
        int init_rpc() = 0;
        int set_value_rpc(int key, string value1<256>, double_vector V_value2) = 1;
        respuesta get_value_rpc(int key) = 2;
        int modify_value_rpc(int key, string value1<256>, double_vector V_value2) = 3;
        int delete_key_rpc(int key) = 4;
        int exist_rpc(int key) = 5;
    } = 1;
} = 2;

