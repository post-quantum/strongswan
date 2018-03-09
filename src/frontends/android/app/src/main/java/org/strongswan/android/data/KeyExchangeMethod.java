package org.strongswan.android.data;


public enum KeyExchangeMethod {

    // DH
    x25519("Curve 25519", "x25519"),
    ecp256("ECP 256", "ecp256"),

    // QS
    qs_sikep503("SIKEp503", "qs_sikep503"),
    qs_ntrukem443("NTRU-KEM-443", "qs_ntrukem443"),
    qs_newhope512cca("New Hope 512 CCA", "qs_newhope512cca"),
    qs_kyber512("KYBER 512", "qs_kyber512"),
    qs_ntrulpr4591761("NTRU PRIME 4591^761", "qs_ntrulpr4591761"),
    qs_ledakem128sln02("LEDA KEM 128SLN02", "qs_ledakem128sln02");

    public final String display;
    public final String value;

    KeyExchangeMethod (String display, String value) {
        this.display = display;
        this.value   = value;
    }

    @Override
    public String toString() {
        return display;
    }

    public static KeyExchangeMethod fromDisplay( String display) {
        if (display != null ) {
            for (KeyExchangeMethod method : KeyExchangeMethod.values() ) {
                if (display.equalsIgnoreCase(method.display)) {
                    return method;
                }
            }
        }
        throw new IllegalArgumentException("No KeyExchangeMethod with display text: " + display);
    }

    public static KeyExchangeMethod fromValue(String value) {
        if (value != null) {
            for (KeyExchangeMethod method : KeyExchangeMethod.values()) {
                if (value.equals(method.value)) {
                    return method;
                }
            }
        }
        throw new IllegalArgumentException("No KeyExchangeMethod with value: " + value);
    }

}

