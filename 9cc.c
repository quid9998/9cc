#include <stdio.h>
#include <stdlib.h>

//argc→コマンドライン引数の個数
//**argv→引数の配列(標準は*argv[]の形で与えられる→意味的には同じ)
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    //strtolは数値を読み込み，第二引数のポインタをアップデートして次の文字を指すように値を更新
    //最後の引数10は基数
    printf("    mov rax, %ld\n", strtol(p, &p, 10));

    while (*p)
    { //配列最後はヌルポインタ
        if (*p == '+')
        {
            p++;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }
        if (*p == '-')
        {
            p++;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "予期しない文字です(構文エラー):%c\n", *p);
        return 1;
    }

    printf("ret\n");
    return 0;
}