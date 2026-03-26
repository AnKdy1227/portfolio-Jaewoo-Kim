#ifndef __LPF_H__
#define __LPF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 초기화 및 필터 업데이트 함수 프로토타입 */
void LPF_Init(void);
double LPF_Update(double input);

#ifdef __cplusplus
}
#endif

#endif /* __LPF_H__ */