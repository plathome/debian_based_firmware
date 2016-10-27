#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <alsa/asoundlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

#define min(a, b)	( (a < b) ? (a) : (b) )

struct WAVE {
	short id;
	short channel;
	int rate;
	int speed;
	short block;
	short bit;
	unsigned int hlen;	/* header size */
	unsigned int len;	/* data size */
};

static snd_pcm_t *handle;

/*
	iead and check wave header
*/
int read_header(char *fname, struct WAVE* wave)
{
	FILE *fp;
	char top[12], buf[4];
	unsigned char tmp[4];
	int len;
	unsigned char* p;

	if((fp = fopen(fname, "r")) == NULL){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		return -1;
	}

	/* read top 12 bytes */
	if(fread(top, 1, 12, fp) != 12){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		fclose(fp);
		return -1;
	}

	/* check header string */
	if(strncmp(&top[0], "RIFF", 4) != 0){
		printf("%d: %s is not WAVE file\n", __LINE__, fname);
		fclose(fp);
		return -1;
	}
	if(strncmp(&top[8], "WAVE", 4) != 0){
		printf("%d: %s is not WAVE file\n", __LINE__, fname);
		fclose(fp);
		return -1;
	}

	/* search "fmt " */
	while(1){
		if(fread(buf, 1, 4, fp) != 4){
			printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
			fclose(fp);
			return -1;
		}
		if(strncmp(buf, "fmt ", 4) == 0){
			break;
		}
	}
	/* read fmt size */
	if(fread(buf, 1, 4, fp) != 4){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		fclose(fp);
		return -1;
	}
	memcpy(tmp, buf, sizeof(buf));
	len = tmp[0] + (tmp[1] << 8) + (tmp[2] << 16) + (tmp[3] << 24);
	if((p = malloc(len)) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}

	/* read fmt data */
	if(fread(p, 1, len, fp) != len){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		fclose(fp);
		return -1;
	}

	/* set fmt data */
	wave->id = p[0] + (p[1] << 8);
	wave->channel = p[2] + (p[3] << 8);
	wave->rate = (p[4] + (p[5] << 8) + (p[6] << 16) + (p[7] << 24)) & 0xffff;
	wave->speed = p[8] + (p[9] << 8) + (p[10] << 16) + (p[11] << 24);
	wave->block = p[12] + (p[13] << 8);
	wave->bit = p[14] + (p[15] << 8);
	free(p);

	/* search "data" */
	while(1){
		if(fread(buf, 1, 4, fp) != 4){
			printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
			fclose(fp);
			return -1;
		}
		if(strncmp(buf, "data", 4) == 0){
			break;
		}
	}
	/* read data size */
	if(fread(buf, 1, 4, fp) != 4){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		fclose(fp);
		return -1;
	}
	memcpy(tmp, buf, sizeof(buf));
	wave->len = tmp[0] + (tmp[1] << 8) + (tmp[2] << 16) + (tmp[3] << 24);
	wave->hlen = ftell(fp);

	fclose(fp);
	return 0;
}

int get_sample_bit(short bit)
{
	int ret = SND_PCM_FORMAT_UNKNOWN;

	switch(bit){
	case 8:
		ret = SND_PCM_FORMAT_S8;
		break;
	case 16:
		ret = SND_PCM_FORMAT_S16;
		break;
	case 24:
		ret = SND_PCM_FORMAT_S24;
		break;
	case 32:
		ret = SND_PCM_FORMAT_S32;
		break;
	default:
		ret = SND_PCM_FORMAT_UNKNOWN;
		break;
	}
	return ret;
}

void control_volume(char *vol)
{
	long min, max;
	snd_mixer_t *mixer;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "PCM";
	long volume = strtol(vol, NULL, 10);

	snd_mixer_open(&mixer, 0);
	snd_mixer_attach(mixer, card);
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t* elem = snd_mixer_find_selem(mixer, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

	snd_mixer_close(mixer);
}

int play_wave(char* fname, char *vol, struct WAVE* wave)
{
	FILE *fp;
	unsigned char *p;
	int i, ret;
	int frm, frms;

	if((fp = fopen(fname, "r")) == NULL){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		return -1;
	}

	/* seek data pointer  */
	fseek(fp, wave->hlen, SEEK_SET);

	if((p = malloc(wave->len)) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	if(fread(p, 1, wave->len, fp) != wave->len){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), fname);
		free(p);
		fclose(fp);
		return -1;
	}
	fclose(fp);

	/* todo: */
	if(snd_pcm_open(&handle, "hw:0", SND_PCM_STREAM_PLAYBACK, 0) < 0){
		printf("%d: %s(default)\n", __LINE__, strerror(errno));
		free(p);
		return -1;
	}

	if(snd_pcm_set_params(handle, 
			get_sample_bit(wave->bit),
			SND_PCM_ACCESS_RW_INTERLEAVED,
			wave->channel,
			wave->rate,
			1,
			50000)){
		printf("%d: %s(default)\n", __LINE__, strerror(errno));
		free(p);
		snd_pcm_close(handle);
		return -1;
	}

	frm = snd_pcm_format_size(get_sample_bit(wave->bit), wave->channel);

	i=0;
	frms=0;
	while(frm < wave->len - i){
		frms = min(wave->rate >> 1, (wave->len - i) / frm);
		if((ret = snd_pcm_writei(handle, &p[i], frms)) < 0){
			snd_pcm_recover(handle, ret, 0);
			ret = 0;
		}
		i += ret * frm;
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	handle=0;
	free(p);
	return 0;
}

int main(int ac, char *av[])
{
	struct WAVE wave;

	if(ac != 2 && ac != 3){
		printf("%s wavefile [volume]\n", av[0]);
		printf("volume : 0 - 100\n");
		return 0;
	}

	memset(&wave, 0x0, sizeof(struct WAVE));
	if(read_header(av[1], &wave) == -1){
		return -1;
	}
	printf("%s : %d Hz %d bit %s\n", av[1], wave.rate, wave.bit,
					(wave.channel == 1) ? "mono" : "stereo");

	if(av[2] != NULL){
		control_volume(av[2]);
	}
	if(play_wave(av[1], av[2], &wave) == -1){
		return -1;
	}

	return 0;
}
