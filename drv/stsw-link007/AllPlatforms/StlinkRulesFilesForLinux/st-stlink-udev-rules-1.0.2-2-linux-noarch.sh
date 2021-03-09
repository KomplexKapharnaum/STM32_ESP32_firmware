#!/bin/sh
# This script was generated using Makeself 2.2.0

umask 077

CRCsum="1679344230"
MD5="f3c4a3a4aae65f88a4a3bee165d42306"
TMPROOT=${TMPDIR:=/tmp}

label="STM udev rules installer"
script="./setup.sh"
scriptargs="1.0.2-2"
licensetxt=""
targetdir="root"
filesizes="6673"
keep="y"
quiet="n"

print_cmd_arg=""
if type printf > /dev/null; then
    print_cmd="printf"
elif test -x /usr/ucb/echo; then
    print_cmd="/usr/ucb/echo"
else
    print_cmd="echo"
fi

unset CDPATH

MS_Printf()
{
    $print_cmd $print_cmd_arg "$1"
}

MS_PrintLicense()
{
  if test x"$licensetxt" != x; then
    echo $licensetxt
    while true
    do
      MS_Printf "Please type y to accept, n otherwise: "
      read yn
      if test x"$yn" = xn; then
        keep=n
 	eval $finish; exit 1        
        break;    
      elif test x"$yn" = xy; then
        break;
      fi
    done
  fi
}

MS_diskspace()
{
	(
	if test -d /usr/xpg4/bin; then
		PATH=/usr/xpg4/bin:$PATH
	fi
	df -kP "$1" | tail -1 | awk '{ if ($4 ~ /%/) {print $3} else {print $4} }'
	)
}

MS_dd()
{
    blocks=`expr $3 / 1024`
    bytes=`expr $3 % 1024`
    dd if="$1" ibs=$2 skip=1 obs=1024 conv=sync 2> /dev/null | \
    { test $blocks -gt 0 && dd ibs=1024 obs=1024 count=$blocks ; \
      test $bytes  -gt 0 && dd ibs=1 obs=1024 count=$bytes ; } 2> /dev/null
}

MS_dd_Progress()
{
    if test "$noprogress" = "y"; then
        MS_dd $@
        return $?
    fi
    file="$1"
    offset=$2
    length=$3
    pos=0
    bsize=4194304
    while test $bsize -gt $length; do
        bsize=`expr $bsize / 4`
    done
    blocks=`expr $length / $bsize`
    bytes=`expr $length % $bsize`
    (
        dd bs=$offset count=0 skip=1 2>/dev/null
        pos=`expr $pos \+ $bsize`
        MS_Printf "     0%% " 1>&2
        if test $blocks -gt 0; then
            while test $pos -le $length; do
                dd bs=$bsize count=1 2>/dev/null
                pcent=`expr $length / 100`
                pcent=`expr $pos / $pcent`
                if test $pcent -lt 100; then
                    MS_Printf "\b\b\b\b\b\b\b" 1>&2
                    if test $pcent -lt 10; then
                        MS_Printf "    $pcent%% " 1>&2
                    else
                        MS_Printf "   $pcent%% " 1>&2
                    fi
                fi
                pos=`expr $pos \+ $bsize`
            done
        fi
        if test $bytes -gt 0; then
            dd bs=$bytes count=1 2>/dev/null
        fi
        MS_Printf "\b\b\b\b\b\b\b" 1>&2
        MS_Printf " 100%%  " 1>&2
    ) < "$file"
}

MS_Help()
{
    cat << EOH >&2
Makeself version 2.2.0
 1) Getting help or info about $0 :
  $0 --help   Print this message
  $0 --info   Print embedded info : title, default target directory, embedded script ...
  $0 --lsm    Print embedded lsm entry (or no LSM)
  $0 --list   Print the list of files in the archive
  $0 --check  Checks integrity of the archive
 
 2) Running $0 :
  $0 [options] [--] [additional arguments to embedded script]
  with following options (in that order)
  --confirm             Ask before running embedded script
  --quiet		Do not print anything except error messages
  --noexec              Do not run embedded script
  --keep                Do not erase target directory after running
			the embedded script
  --noprogress          Do not show the progress during the decompression
  --nox11               Do not spawn an xterm
  --nochown             Do not give the extracted files to the current user
  --target dir          Extract directly to a target directory
                        directory path can be either absolute or relative
  --tar arg1 [arg2 ...] Access the contents of the archive through the tar command
  --                    Following arguments will be passed to the embedded script
EOH
}

MS_Check()
{
    OLD_PATH="$PATH"
    PATH=${GUESS_MD5_PATH:-"$OLD_PATH:/bin:/usr/bin:/sbin:/usr/local/ssl/bin:/usr/local/bin:/opt/openssl/bin"}
	MD5_ARG=""
    MD5_PATH=`exec <&- 2>&-; which md5sum || type md5sum`
    test -x "$MD5_PATH" || MD5_PATH=`exec <&- 2>&-; which md5 || type md5`
	test -x "$MD5_PATH" || MD5_PATH=`exec <&- 2>&-; which digest || type digest`
    PATH="$OLD_PATH"

    if test "$quiet" = "n";then
    	MS_Printf "Verifying archive integrity..."
    fi
    offset=`head -n 500 "$1" | wc -c | tr -d " "`
    verb=$2
    i=1
    for s in $filesizes
    do
		crc=`echo $CRCsum | cut -d" " -f$i`
		if test -x "$MD5_PATH"; then
			if test `basename $MD5_PATH` = digest; then
				MD5_ARG="-a md5"
			fi
			md5=`echo $MD5 | cut -d" " -f$i`
			if test $md5 = "00000000000000000000000000000000"; then
				test x$verb = xy && echo " $1 does not contain an embedded MD5 checksum." >&2
			else
				md5sum=`MS_dd "$1" $offset $s | eval "$MD5_PATH $MD5_ARG" | cut -b-32`;
				if test "$md5sum" != "$md5"; then
					echo "Error in MD5 checksums: $md5sum is different from $md5" >&2
					exit 2
				else
					test x$verb = xy && MS_Printf " MD5 checksums are OK." >&2
				fi
				crc="0000000000"; verb=n
			fi
		fi
		if test $crc = "0000000000"; then
			test x$verb = xy && echo " $1 does not contain a CRC checksum." >&2
		else
			sum1=`MS_dd "$1" $offset $s | CMD_ENV=xpg4 cksum | awk '{print $1}'`
			if test "$sum1" = "$crc"; then
				test x$verb = xy && MS_Printf " CRC checksums are OK." >&2
			else
				echo "Error in checksums: $sum1 is different from $crc" >&2
				exit 2;
			fi
		fi
		i=`expr $i + 1`
		offset=`expr $offset + $s`
    done
    if test "$quiet" = "n";then
    	echo " All good."
    fi
}

UnTAR()
{
    if test "$quiet" = "n"; then
    	tar $1vf - 2>&1 || { echo Extraction failed. > /dev/tty; kill -15 $$; }
    else

    	tar $1f - 2>&1 || { echo Extraction failed. > /dev/tty; kill -15 $$; }
    fi
}

finish=true
xterm_loop=
noprogress=n
nox11=n
copy=none
ownership=y
verbose=n

initargs="$@"

while true
do
    case "$1" in
    -h | --help)
	MS_Help
	exit 0
	;;
    -q | --quiet)
	quiet=y
	noprogress=y
	shift
	;;
    --info)
	echo Identification: "$label"
	echo Target directory: "$targetdir"
	echo Uncompressed size: 16 KB
	echo Compression: gzip
	echo Date of packaging: Tue Sep 24 11:44:18 UTC 2019
	echo Built with Makeself version 2.2.0 on 
	echo Build command was: "/usr/bin/makeself \\
    \"--notemp\" \\
    \"/src/work/udev_rules_installer/makeself/root\" \\
    \"/src/work/udev_rules_installer/makeself/st-stlink-udev-rules-1.0.2-2-linux-noarch.sh\" \\
    \"STM udev rules installer\" \\
    \"./setup.sh\" \\
    \"1.0.2-2\""
	if test x$script != x; then
	    echo Script run after extraction:
	    echo "    " $script $scriptargs
	fi
	if test x"" = xcopy; then
		echo "Archive will copy itself to a temporary location"
	fi
	if test x"y" = xy; then
	    echo "directory $targetdir is permanent"
	else
	    echo "$targetdir will be removed after extraction"
	fi
	exit 0
	;;
    --dumpconf)
	echo LABEL=\"$label\"
	echo SCRIPT=\"$script\"
	echo SCRIPTARGS=\"$scriptargs\"
	echo archdirname=\"root\"
	echo KEEP=y
	echo COMPRESS=gzip
	echo filesizes=\"$filesizes\"
	echo CRCsum=\"$CRCsum\"
	echo MD5sum=\"$MD5\"
	echo OLDUSIZE=16
	echo OLDSKIP=501
	exit 0
	;;
    --lsm)
cat << EOLSM
No LSM.
EOLSM
	exit 0
	;;
    --list)
	echo Target directory: $targetdir
	offset=`head -n 500 "$0" | wc -c | tr -d " "`
	for s in $filesizes
	do
	    MS_dd "$0" $offset $s | eval "gzip -cd" | UnTAR t
	    offset=`expr $offset + $s`
	done
	exit 0
	;;
	--tar)
	offset=`head -n 500 "$0" | wc -c | tr -d " "`
	arg1="$2"
    if ! shift 2; then MS_Help; exit 1; fi
	for s in $filesizes
	do
	    MS_dd "$0" $offset $s | eval "gzip -cd" | tar "$arg1" - $*
	    offset=`expr $offset + $s`
	done
	exit 0
	;;
    --check)
	MS_Check "$0" y
	exit 0
	;;
    --confirm)
	verbose=y
	shift
	;;
	--noexec)
	script=""
	shift
	;;
    --keep)
	keep=y
	shift
	;;
    --target)
	keep=y
	targetdir=${2:-.}
    if ! shift 2; then MS_Help; exit 1; fi
	;;
    --noprogress)
	noprogress=y
	shift
	;;
    --nox11)
	nox11=y
	shift
	;;
    --nochown)
	ownership=n
	shift
	;;
    --xwin)
	finish="echo Press Return to close this window...; read junk"
	xterm_loop=1
	shift
	;;
    --phase2)
	copy=phase2
	shift
	;;
    --)
	shift
	break ;;
    -*)
	echo Unrecognized flag : "$1" >&2
	MS_Help
	exit 1
	;;
    *)
	break ;;
    esac
done

if test "$quiet" = "y" -a "$verbose" = "y";then
	echo Cannot be verbose and quiet at the same time. >&2
	exit 1
fi

MS_PrintLicense

case "$copy" in
copy)
    tmpdir=$TMPROOT/makeself.$RANDOM.`date +"%y%m%d%H%M%S"`.$$
    mkdir "$tmpdir" || {
	echo "Could not create temporary directory $tmpdir" >&2
	exit 1
    }
    SCRIPT_COPY="$tmpdir/makeself"
    echo "Copying to a temporary location..." >&2
    cp "$0" "$SCRIPT_COPY"
    chmod +x "$SCRIPT_COPY"
    cd "$TMPROOT"
    exec "$SCRIPT_COPY" --phase2 -- $initargs
    ;;
phase2)
    finish="$finish ; rm -rf `dirname $0`"
    ;;
esac

if test "$nox11" = "n"; then
    if tty -s; then                 # Do we have a terminal?
	:
    else
        if test x"$DISPLAY" != x -a x"$xterm_loop" = x; then  # No, but do we have X?
            if xset q > /dev/null 2>&1; then # Check for valid DISPLAY variable
                GUESS_XTERMS="xterm rxvt dtterm eterm Eterm kvt konsole aterm"
                for a in $GUESS_XTERMS; do
                    if type $a >/dev/null 2>&1; then
                        XTERM=$a
                        break
                    fi
                done
                chmod a+x $0 || echo Please add execution rights on $0
                if test `echo "$0" | cut -c1` = "/"; then # Spawn a terminal!
                    exec $XTERM -title "$label" -e "$0" --xwin "$initargs"
                else
                    exec $XTERM -title "$label" -e "./$0" --xwin "$initargs"
                fi
            fi
        fi
    fi
fi

if test "$targetdir" = "."; then
    tmpdir="."
else
    if test "$keep" = y; then
	if test "$quiet" = "n";then
	    echo "Creating directory $targetdir" >&2
	fi
	tmpdir="$targetdir"
	dashp="-p"
    else
	tmpdir="$TMPROOT/selfgz$$$RANDOM"
	dashp=""
    fi
    mkdir $dashp $tmpdir || {
	echo 'Cannot create target directory' $tmpdir >&2
	echo 'You should try option --target dir' >&2
	eval $finish
	exit 1
    }
fi

location="`pwd`"
if test x$SETUP_NOCHECK != x1; then
    MS_Check "$0"
fi
offset=`head -n 500 "$0" | wc -c | tr -d " "`

if test x"$verbose" = xy; then
	MS_Printf "About to extract 16 KB in $tmpdir ... Proceed ? [Y/n] "
	read yn
	if test x"$yn" = xn; then
		eval $finish; exit 1
	fi
fi

if test "$quiet" = "n";then
	MS_Printf "Uncompressing $label"
fi
res=3
if test "$keep" = n; then
    trap 'echo Signal caught, cleaning up >&2; cd $TMPROOT; /bin/rm -rf $tmpdir; eval $finish; exit 15' 1 2 3 15
fi

leftspace=`MS_diskspace $tmpdir`
if test -n "$leftspace"; then
    if test "$leftspace" -lt 16; then
        echo
        echo "Not enough space left in "`dirname $tmpdir`" ($leftspace KB) to decompress $0 (16 KB)" >&2
        if test "$keep" = n; then
            echo "Consider setting TMPDIR to a directory with more free space."
        fi
        eval $finish; exit 1
    fi
fi

for s in $filesizes
do
    if MS_dd_Progress "$0" $offset $s | eval "gzip -cd" | ( cd "$tmpdir"; UnTAR x ) 1>/dev/null; then
		if test x"$ownership" = xy; then
			(PATH=/usr/xpg4/bin:$PATH; cd "$tmpdir"; chown -R `id -u` .;  chgrp -R `id -g` .)
		fi
    else
		echo >&2
		echo "Unable to decompress $0" >&2
		eval $finish; exit 1
    fi
    offset=`expr $offset + $s`
done
if test "$quiet" = "n";then
	echo
fi

cd "$tmpdir"
res=0
if test x"$script" != x; then
    if test x"$verbose" = xy; then
		MS_Printf "OK to execute: $script $scriptargs $* ? [Y/n] "
		read yn
		if test x"$yn" = x -o x"$yn" = xy -o x"$yn" = xY; then
			eval $script $scriptargs $*; res=$?;
		fi
    else
		eval $script $scriptargs $*; res=$?
    fi
    if test $res -ne 0; then
		test x"$verbose" = xy && echo "The program '$script' returned an error code ($res)" >&2
    fi
fi
if test "$keep" = n; then
    cd $TMPROOT
    /bin/rm -rf $tmpdir
fi
eval $finish; exit $res
� ��]�TgP�k�2Y	"CNC$Ð%'�a�����!�d9#�$A����d$K�Y$�  ��޻{���٪o�v�S�[�V�y����Ӽ|�9��qWD�/ pWX�����]ѻ¿�   $",(��/*(x���D ��@��� ���­�9�����M���|((�Í����_���/ r����������/3#�̕���zB�(�U�E �كLA<� &�?�L s�$�u@m i6A����:�� ���� 9��
s��.n��v�2]��A@{��CQ�n�hG)>(ږ�������k�',���K����3���?��\@�#�}�B[�t�%[��l���ׄ�A��??����_8jm��g~P;�=��
0��j��G A�7�CO�R[]��PIWOU�>��P
�H����u��a����K@W(� �@�r������a�� ��3Pā�����!��\�����P�H@��������O���3�b7���̹�A�|ϙ��	sE�X����������nz�# '�g���rDx��~օ�@�l�K��]3~��R	~������6Mj�i��@��/$�ש��H^0��uGA�����d����	���1� 0���|@v�_}�[�?�����׊��p��3��Y���'.�k��7�5�������^��������s���+[���W�G��?������d�	�
�}��		����`���c���v͜c�&<���	bc�2�hD�Y���!�,e��0�����O�3�$�Rv���+^-�ǅ�q���0v�"}�mVI(��s��� �'
w'tW�t��b�L�Ch}.��Ko���ޛ;�8=�v�Zūu�����pD�p������n;O�B�מߋ��' ,\���]5ۋ_^��;���S��r���B�
�r�Ki�"���Q�e��e��ΰbЄ+v��s�o}y�@kDT��ޙ��E?����p�|�)�8L��l���������A�De��@�
�O�HI&CMD�wψ�B�J�剔{��)Î�V�|��ҹ�u�=cY3w�|~�����h������X���v���y�W��A�$�����X7�KĄ0�?&o���<t�rx�FvN9�ʫ�5�yZ����Sys-���$���B�a�WQޫ��V��wvBJ�����i% Q�/l�U��h-Ĝ��Ǖ�+4�
��U��v��1~���,n,��Ls�u��T�4��7YƗ9��p3>�l.o0�����[.<H�Oy�|歷Ć�����)��.ݘ�Òc�=&-�������4
���}	��d.2l�V̈́�KU��r�w#��H%HlFֺ�1=L�y�|���m�4�1rR�WL�2m����L(��6�Z~n`�Nim�
�e��Ʈ�/�_��u1�h�η�) �a�IF[��E9�tTP���mt�
�$n/EGq6�T�J�Ǹs�z�-������訂��GR%6���F+�sxR��)��eC���Kr�
N�3�k$�	��ml���};��>֎!Q5�S�h��7B��ug|����=ro�`�Ԃ�7*��m���(���	����(���b����p+�w���V���S��w��k-!uju��֍�5���[OWθ-�v��ͣ�����Gh�0�WDl(�䱕�9čp�X}�Ɵ�'�~�j����%x
�gʬ��R-����.�v. e���|y�Y���J��y�l5n��Ȫ�WF�x�pF��O�ɵ�7�����h?���ifY��QM������E��]�1L�nL�T8�ۤ�]�v��N��4�BfR�Uv��Zŏ>�ź+φĜ��>�O�4c�idi0k4�H�e��y�TҶ�A��۟�T��Y�k00���jP��5��L�D�L��4s���}kk���`|֋W~�P��I��ɫ|�К�'��(�}��IG�_����5��Wt�� ����� m��d x{�8a.Kw��w�M��%�')9��,ߺC�\mA�B�\}i�������mO�|w)����~S�q��{/�j����$�`)�_U�%���IP�sk���B���ϒ�5��9޻�^�[g,]U�Pqd����*�x���݈H
�)-x�X�΅kլBa}ң/á�VD���<���	ur�P��k���N�*����R�Io+09�c��4[8س�Pդ�"-o��&��b\�)�xl���.~�|�!�9Jd�*bd�,Mܜ� cM�!X���'c:�~t���]e�
������B�7'�S��񪈵��x�/m��\�Yw��&\��k��0B���
�n��po�\..��γ޹\��Vv�'q�������L�e�7f�O�>rx�dKUa�6��< �Q��Q����ç����#���V���Vl׎���-�E���'V��z1v���_H�B�o�Ӟ�g�x�� ��3V�^0�t�s!3HÞ���[�����堘1�խ�I��p�C��R�y8;��z��Q���#�5��Ml3�%"��p7����I�q�(_{�����̉���?.�?2�3�MH����Q���<E�S-ȶJq���e&�Tl��LU�H,���		$�h��)' H�% 
�xV�Jʨ�:Cn�1��P���f|��i����=���Mhb�n���ozyH��G��CJ�Ki0bg���`�����Bd�I_�n�Q�\XG��u|w�"�/�y��k���t)��\N�F?p�6��Ѱ)����]k�ҷ�������\7��T��Ē3���m-b�Ҥ��@�`Z{���mcd{��d����_�}�h���L���_R�fS��M�4�a��'4S���H��B�@V��
�5 �W|s�>�!:G��R�XN7�������"�"�e�$�g3�4����� ��S����Y8X����Bu:Ŏ��:ܡ#V[�hn|�� ޓ}��bkH�+zB�_R5��OQg텏�f����&�m��*k�|��S�Xö�vRmvMY���pc��9$�ᙕ���d1�H�X���ڱr_�.�'dj�|�$�2�'�Qï	�������}|^L5T`;�#+c��T�|@�ڋ������\���>����8�����h1/�i�-�\����u;��*�w���h�`��}�3�z�?�!�g��㓃���ܔ&��SS�%���q}0ս����x�z"{�y2���&r�q�^��J�������%&�3<Fx����0��[�RﷷE�UBb�VC�0���p����=��.����z`�$�$#4�(g^/d�آ�p#}UtkƁP���T�!�/����/P�=&�7���L	�o��o�6&޿ύa��Rgkzg8RME��q���H��|�a�Ȥ�G,���q~=X3���sm�n�� �zn`�]W�F�Af��;�ubV,��6Kj�<S�[��A�A��3�؏�$*�&
0f� 꼶&䘢}Ry���^����p�}Y��y߳x_`[āK��$�c�[���w71�u#<�:�w3��N��8������B�Qzz2G��D�92�Y�uF��%��_���%A�T�ԔLN%a�9́J�r�[�)�t�\3$A�
s��Q��n8_� '��<��̈́��i�zCqlh�Sg�:��Mɕk��xR��ݻ=:6sØ`��Է0�廷���޸��旿�3#c��Qa��z�����B���5͠�iے+����2,8���w���N^Q�˅�@�+��ƙ3����z�uz�o��4�43��F�b�L��T���e���ި���Nj ��	t�#I+�k�.���՗B��)-���S���&���y&�l	�Z#�3p@E�nɤ�v}�C�7a6Je$A�����S!�6Y�Oڨ9gh�eYn�Wxņ��[�e(��p�<3ju�P��2lu<��o�5�ђK���ݘD��]W[�&R�s�,�<�E&�>+��G �t��b�%��z�l�8NR*����1b��}�ު�촯�%�|Hw'��P���&�J�M��P"��1WA���S�!��(��*'y�L���Q�~�O�S��F��,�f0���mkC�C�PqV�C�:Lj�H7Eh���^?����}7DH1c�FlY��@���y�ݡ<'�R@OX�X�4.��WZ�i�0�Y��H�$K���|��s݄>G��Vvg���ɬOjt��<U���'s�Vky�@Yr�����ԣ3�Y �0KeyٚW�}+�Ѓ�i�,/����ə�}�o6�����w��]D�4־�x�H��B�&V��^�j=ӂ��kI����!Ӑ��dl}p���Uhy�_f�����YbIB![c�r�iEk��u�^/uU�l�F2� v�\A���tֱ���*��-�1m͗���T� ��O�o(�݄%����g�����5��;�GQ�D���d�0��-@���6[44�RM��`�y��yW	��Y�M�̰f\A�]�pI	B@�)*�-�V�l�h��ś�*�Ún��P�e�0_#��'d	�+�riFDN�߸���'����H^��N�1Ni$&%`>����'�8��f��+@�΀�3���3��ݨvqs��c��y�w�nl�i<]r+�),|HKJt䘰��I��1;\�t	���LB���6V4z��iH�.	ַi����$����rLQA]�ю܎s�«� �v�>�@~c)���P;K�@6��ݮuu�)�AD�	`(���xه����������p�$⇒��!{��
%aBx H8%�c)n�\���m���C�k���h����N#[�@�RP_g��V�PV�Ă.씨\� o�X\{��x#���^Rه5�}����l4W�o"�Ģ�p�CS�i�Yؕ��?r伋��Ԥ��vI�zV�]@�5��-�ۤD�};�		�L�m���Z��^�jkr�mb��BH?��Q|���`jF�9Rz����,!KŢ1�C=V��}l����z�XH�Ơ��r�J b{DZi2�?�՟XK�?����� ���'Tn��3Ȟ��}Jv$O��2����k��L�>�`�������_��+���T�<��喞a�=�ۗ�/d9v�+�����;�o;*��΂$/�;�w%�v=v��G�nq���ƕ��oZ�a�g���Kh;q�pm��w��$z�M�_����t�<�bQ|I�n�i��q�lSQ����<¤��y2N�L��6�O�������@g5q��<����S]��<�Y'a��m�;�I-Y�'�ZuU%�N?k��ZÄz�9D%#��u����߸ˏ:KC�0�·�\���1� ��eaإN�Z�j�6��;x�͌OW+�7~�]�f[ﯮ�OhS��s��6���C��֣I��`~�����M�ߵ�����L����ψ,^��b<H�����d!$?���'F�/Y��/0�m�̞DV��u���G�	��㸗s�h!��Ǘ�r�
-��H%�|�ʝ�?̈́.\��?�3�[�̌�Pp�ۿJ���l�	�"hj�__�s��cK�βqP;pMO����OpLI�s�4_�I�$+��L�=���9]��jo���~ݎ�A���cO�3�_ׂs(So�S�e{$��������u->n��ʲ�t�ݶ����~? �X���������{�G�N.c��.�g�����?Ѫ}{¯XR1Mh�sU!*ż�_Z"32�IN�E5��^v�Ty�uw�H�ѳi�OG�:�FxJ���{�:�C�.��I�Õ��������2��T������ƥ?�����_E�MF�,���d��ꢾ�q�۰��D�q7筈��>���'H��W������;�]Ŵ�k����M[�V(����Qi>�������~��Z�����2���g=�=cɋ=��EG���ҫ,}Wm�饮�^dJ��M4��/ye�����\���Z�_��m�9#3�>�*�8�l`��b=�uyG{�TUv��`��lJɷ���e���IO3��3//P�~������(w:��l�84%A�����d����߷��,}��������s���xo�Ϳ5`��d"��^�w��������ocIʖe�$Q+�2�~?&���UH!��>�IL����>�3$$#tܦ�Uܴy���G����؆/��Qw��u��f�|˙ZP3�z�Ewna,���d��8<l�~� �<�i�>S_\�L���;�:M�!i�X˓���8<�)��t�Tby����<�/�/�&cH�S�HZ2[�t�=&�i��:���:��h8d0\8ɧ�]���u��r�ņ��4u��o9-�������psi\d���S�7���6�h}}_������W/�N3��|�1�;8���}�8_S'����IV�,�kZez�糜R,���6�ek'߲y�=m�[�%р��|?�`{x�n'��K����Ks����ū�����/Z����Ag��%�jp�^�J��K��Q���3u_a
{f@ v������������7�d"� (  