
#include "hls_config.h"

void hls_pp(data_t *pp, data_t x[N], data_t y[N])
{
#pragma HLS INTERFACE mode=s_axilite port=x storage_impl=bram //se puede omitir la forma de almacenamiento
#pragma HLS INTERFACE mode=s_axilite port=y storage_impl=bram
#pragma HLS INTERFACE mode=s_axilite port=pp
#pragma HLS INTERFACE mode=s_axilite port=return //indica que tenga pines de control, ya que es un void (la funcion), si fuera de algun tipo distinto de void habria un puerto fisico

#pragma HLS ARRAY_PARTITION variable=x type=cyclic factor=2
#pragma HLS ARRAY_PARTITION variable=y type=cyclic factor=2

	data_t res[N];
	data_t r=0;
	MainLoop: for (int i = 0; i < N; ++i)
	{
		#pragma HLS UNROLL
		res[i] = x[i]*y[i];
	}

	for (int i = 0; i < N; ++i)
		{
			#pragma HLS UNROLL
			r+= res[i];
		}

	*pp = r;
}
